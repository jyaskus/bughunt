#include "Timers.h"


// Generic base timer class. It leaves the real-time counting functions up to derived classes.
// Constructor that initializes the class variables
GenericTimer::GenericTimer(void)
{
	// Initialize the class state variables
	ResetClassStateVariables();
	// Reset (initialize) the statistics variables
	ResetStatistics();
}

// Starts the timer with an initial elapsed time count, instead of starting at 0 - If the timer is already started it simply returns the current time elapsed - If the timer is paused, it simply unpauses it and ignores InitialElapsedTime
double GenericTimer::Start(const double InitialElapsedTime)
{
	TS.ObtainLock();

	// if the timer is not started or if it's paused, get it going
	// otherwise, don't do anything
	if(IsStarted == false || IsPaused == true)
	{
		// Adjust the starting time if the timer isn't currently paused, and if InitialElapsedTime is greater than or equal to 0.0f
		if(IsPaused == false && InitialElapsedTime > 0.0f)
			CurrentElapsedTimeCount = InitialElapsedTime;

		// Set the booleans and start the timer
		IsPaused = false;
		IsStarted = true;

		try
		{
			InitiateTimer();
		}
		catch(exception *e)
		{
			TS.ReleaseLock();
			throw e;
		}
	}

	double TempElapsedTime = 0.0f;
	
	try
	{
		TempElapsedTime=GetElapsedTime();
	}
	catch(exception *e)
	{
		TS.ReleaseLock();
		throw e;
	}

	TS.ReleaseLock();

	// Return the current elapsed time
	return TempElapsedTime;
}

// Pauses or Unpauses the timer
double GenericTimer::Pause(void)
{
	TS.ObtainLock();

	// If it's paused, reinitiate the ticker and reset the IsPaused variable
	if(IsPaused == true)
	{
		try
		{
			InitiateTimer();
		}
		catch(exception *e)
		{
			TS.ReleaseLock();
			throw e;
		}

		IsPaused = false;
	}
	else if(IsStarted == true) // Else if it's started, stop the ticker, update the elapsed time and set the IsPaused variable
	{
		try
		{
			CurrentElapsedTimeCount += HaltTimer();
		}
		catch(exception *e)
		{
			TS.ReleaseLock();
			throw e;
		}

		IsPaused = true;
	}

	// If it's stopped, just do nothing

	double TempElapsedTime = CurrentElapsedTimeCount;
	TS.ReleaseLock();

	// Return the CurrentElapsedTimeCount;
	return TempElapsedTime;
}

// Halts the timer, resets the class variables, then returns the time elapsed in seconds (after adding it to the statistics) - If the timer is already stopped, it does nothing
double GenericTimer::Stop(void)
{
	TS.ObtainLock();

	// If the timer is not running, don't do anything
	if(IsStarted == false)
	{
		double TempElapsedTime = CurrentElapsedTimeCount;
		TS.ReleaseLock();
		return TempElapsedTime; // this should always be 0.0f
	}

	// Get the final time count
	double TempElapsedTime = 0.0f;
	
	try
	{
		TempElapsedTime = CurrentElapsedTimeCount + HaltTimer();
	}
	catch(exception *e)
	{
		TS.ReleaseLock();
		throw e;
	}

	try
	{
		ResetClassStateVariables();
		AddTimeToStatistics(TempElapsedTime);
	}
	catch(exception *e)
	{
		TS.ReleaseLock();
		throw e;
	}
	
	TS.ReleaseLock();

	// Return the final time count, after adding it into the statistics
	return TempElapsedTime;
}

// Get the current elapsed time
double GenericTimer::GetElapsedTime(void)
{
	TS.ObtainLock();
	double TempElapsedTime = 0.0f;

	// if the timer is stopped or paused
	if(IsStarted == false || IsPaused==true)
	{
		TempElapsedTime = CurrentElapsedTimeCount;
	}
	else
	{
		try
		{
			// otherwise, let HaltTimer update CurrentElapsedTimeCount
			TempElapsedTime = CurrentElapsedTimeCount + HaltTimer();
		}
		catch(exception *e)
		{
			TS.ReleaseLock();
			throw e;
		}
	}

	TS.ReleaseLock();
	
	return TempElapsedTime;
}

// Get the minimum time elapsed statistic (note: statistics are filled out each time Stop is called)
double GenericTimer::GetMinimumElapsedTime(void) { TS.ObtainLock(); return MinimumElapsedTime; TS.ReleaseLock(); }

// Get the maximum time elapsed statistic
double GenericTimer::GetMaximumElapsedTime(void) { TS.ObtainLock(); return MaximumElapsedTime; TS.ReleaseLock(); }

// Get the average time elapsed statistic
double GenericTimer::GetAverageElapsedTime(void) { TS.ObtainLock(); return AverageElapsedTime; TS.ReleaseLock(); }

// Reset (initialize) the statistics variables to blanks
void GenericTimer::ResetStatistics(void)
{
	TS.ObtainLock();

	MinimumElapsedTime = 0.0f;
	MaximumElapsedTime = 0.0f;
	AverageElapsedTime = 0.0f;

	try
	{
		StatisticsVector.clear();
	}
	catch(exception *e)
	{
		TS.ReleaseLock();
		throw e;
	}

	StatisticsAreBlank = true;

	TS.ReleaseLock();
}

// Overloaded assignment operator - This is used when the assignment operator of the derived classes are called
GenericTimer& GenericTimer::operator=(const GenericTimer &Source)
{
	TS.ObtainLock();

	CurrentElapsedTimeCount = Source.CurrentElapsedTimeCount;
	IsPaused = Source.IsPaused;
	IsStarted = Source.IsStarted;

	MinimumElapsedTime = Source.MinimumElapsedTime;
	MaximumElapsedTime = Source.MaximumElapsedTime;
	AverageElapsedTime = Source.AverageElapsedTime;
	StatisticsAreBlank = Source.StatisticsAreBlank;
	StatisticsVector = Source.StatisticsVector;

	TS.ReleaseLock();

	return *this;
}

// Reset (initialize) class state variables
void GenericTimer::ResetClassStateVariables(void)
{
	TS.ObtainLock();

	CurrentElapsedTimeCount = 0.0f;
	IsPaused = false;
	IsStarted = false;

	TS.ReleaseLock();
}

// Add a new time entry to the statistics vector
const double& GenericTimer::AddTimeToStatistics(const double &StatisticsEntry)
{
	TS.ObtainLock();

	// Add the time to the vector of times
	try
	{
		StatisticsVector.push_back(StatisticsEntry);
	}
	catch(exception *e)
	{
		TS.ReleaseLock();
		throw e;
	}

	// If the statistics have been recently re-initialized (they are all 0's) then force an update
	if(StatisticsAreBlank)
	{
		MinimumElapsedTime = StatisticsEntry;
		MaximumElapsedTime = StatisticsEntry;
		AverageElapsedTime = StatisticsEntry;
	}
	else
	{
		// Check to see if the statistic is less than the minimum or greater than the maximum
		if(StatisticsEntry < MinimumElapsedTime)
			MinimumElapsedTime = StatisticsEntry;
		else if(StatisticsEntry > MaximumElapsedTime)
			MaximumElapsedTime = StatisticsEntry;

		// Reset AverageElapsedTime before we recalculate it
		AverageElapsedTime = 0.0f;

		try
		{
			// For each entry in the vector, add them all up into AverageElapsedTime
			for(vector<double>::const_iterator i = StatisticsVector.begin(); i != StatisticsVector.end(); i++)
				AverageElapsedTime += *i;

			// Then divide AverageElapsedTime by the number of entries in the vector to obtain the new average
			AverageElapsedTime /= StatisticsVector.size();
		}
		catch(exception *e)
		{
			TS.ReleaseLock();
			throw e;
		}
	}

	// Indicate that the statistics now contain data
	StatisticsAreBlank = false;

	TS.ReleaseLock();

	// Return the same value
	return StatisticsEntry;
}




// Win32Timer class
// Uses GetTickCount, a fairly fast but not extremely accurate timer
// Tests for wraparound - I believe that this class is pretty much bulletproof
// Copy constructor
Win32Timer::Win32Timer(const Win32Timer &Source)
{
	StartingCount=Source.StartingCount;

	// Call the base class's overloaded assignment function so that the base class's data is copied as well
	GenericTimer::operator=(Source);
}

// Overloaded assignment operator
Win32Timer& Win32Timer::operator=(const Win32Timer &Source)
{
	StartingCount = Source.StartingCount;

	// Call the base class's overloaded assignment function so that the base class's data is copied as well
	GenericTimer::operator=(Source);
	return *this;
}

// Sets the start time using the system dependant time counting functions (in 'ticks')
void Win32Timer::InitiateTimer(void)
{
	StartingCount = GetTickCount();
}

// Gets the end time and calculates how much time has elapsed in real seconds since the start, returns that value as a double
double Win32Timer::HaltTimer(void)
{
	DWORD EndingCount = GetTickCount();

	// Calculate elapsed time:
	// First check for wrap back to 0, compensate if need be...
	// Since a DWORD can only hold values up to 4294967295, this means that the 4294967296th millisecond passed since system startup
	// will cause the system timer to wrap back to 0 (occurs every 49.7 days of system uptime - think Windows can hold up that long?) :)
	DWORD MillisecondsElapsed = 0;

	if(EndingCount < StartingCount)
		MillisecondsElapsed = MAXDWORD - StartingCount + EndingCount + 1;
	else
		MillisecondsElapsed = EndingCount - StartingCount;

	return static_cast<double> (MillisecondsElapsed) / 1000;
}



// Win32HighResTimer class
// Fast and accurate
// Doesn't really need to test for wraparound since the variable holding the time is a 64-bit signed int
// Default constructor - this is used to obtain the timer 'tick' frequency (varies among different systems)
Win32HighResTimer::Win32HighResTimer(void)
{
	// Find out how many "ticks" per second occur on this computer
	if(!QueryPerformanceFrequency(&Frequency))
		throw runtime_error("Win32HighResTimer error - QueryPerformanceFrequency()");

	// If the frequency is set to 0, the system doesn't support high res timing using QueryPerformanceCounter
	if(Frequency.QuadPart == 0)
		throw runtime_error("Win32HighResTimer error - This system does not support high performance counters");
}

// Copy constructor
Win32HighResTimer::Win32HighResTimer(const Win32HighResTimer &Source)
{
	Frequency=Source.Frequency;
	StartingCount=Source.StartingCount;

	// Call the base class's overloaded assignment function so that the base class's data is copied as well
	GenericTimer::operator=(Source);
}

// Overloaded assignment operator
Win32HighResTimer& Win32HighResTimer::operator=(const Win32HighResTimer &Source)
{
	Frequency = Source.Frequency; 
	StartingCount = Source.StartingCount;

	// Call the base class's overloaded assignment function so that the base class's data is copied as well
	GenericTimer::operator=(Source); return *this;
}

// Sets the start time using the system dependant time counting functions (in 'ticks')
void Win32HighResTimer::InitiateTimer(void)
{
	// Start the timer
	if(!QueryPerformanceCounter(&StartingCount))
		throw runtime_error("Win32HighResTimer error - QueryPerformanceCounter()");
}

// Gets the end time and calculates how much time has elapsed in real seconds since the start, returns that value as a double
double Win32HighResTimer::HaltTimer(void)
{
	// LARGE_INTEGER union (contains a 64-bit signed int for storing time data) - see msdn.microsoft.com for more details
	LARGE_INTEGER EndingCount;

	// Stop the timer
	if(!QueryPerformanceCounter(&EndingCount))
		throw runtime_error("Win32HighResTimer error - QueryPerformanceCounter()");

	// Calculate elapsed time
	return static_cast<double> (EndingCount.QuadPart - StartingCount.QuadPart) / Frequency.QuadPart;
}