#include <iostream> // for iostreams
#include <vector> // for vector
#include <stdexcept>
#include <windows.h> // for Win32 time functions
using namespace std;


// Class for synchronizing objects in a multi-threaded environment
class Win32ThreadSynchronizer
{
public:

	Win32ThreadSynchronizer(void) { InitializeCriticalSection(&TS_CS); }
	~Win32ThreadSynchronizer(void) { DeleteCriticalSection(&TS_CS); }
	__forceinline void ObtainLock(void) { EnterCriticalSection(&TS_CS); }
	__forceinline void ReleaseLock(void) { LeaveCriticalSection(&TS_CS); }

private:

	CRITICAL_SECTION TS_CS;
};


// Generic base timer class. It leaves the real-time counting functions up to derived classes.
class GenericTimer
{
public:
		
	GenericTimer(void); // Constructor that initializes the class variables

	// Start/Pause/Stop functions - They all return the current time elapsed in seconds as a double
	double Start(const double InitialElapsedTime = 0.0f); // Starts the timer (with an optional initial elapsed time count, default is 0) - If the timer is already started it simply returns the current time elapsed - If the timer is paused, it simply unpauses it and ignores InitialElapsedTime
	double Pause(void); // Pauses or Unpauses the timer
	double Stop(void); // Halts the timer, resets the class variables, then returns the time elapsed in seconds (after adding it to the statistics) - If the timer is already stopped, it does nothing

	// For enumerating/resetting statistics
	double GetElapsedTime(void); // Get the current elapsed time
	double GetMinimumElapsedTime(void); // Get the minimum time elapsed statistic (note: statistics are filled out each time Stop is called)
	double GetMaximumElapsedTime(void); // Get the maximum time elapsed statistic
	double GetAverageElapsedTime(void); // Get the average time elapsed statistic
	void ResetStatistics(void); // Reset (initialize) the statistics variables to blanks
	friend ostream& GenericTimer::operator << (ostream &outstream, GenericTimer &Source) // Overloaded << operator for printing out statistics to a stream
	{
		outstream << "Current Time: " << Source.GetElapsedTime() << "s" << endl;
		outstream << "Minimum Time: " << Source.GetMinimumElapsedTime() << "s" << endl;
		outstream << "Maximum Time: " << Source.GetMaximumElapsedTime() << "s" << endl;
		outstream << "Average Time: " << Source.GetAverageElapsedTime() << "s" << endl;

		return outstream;
	}

protected:

	// Overloaded assignment operator - This is used when the assignment operator of the derived classes are called
	GenericTimer &operator=(const GenericTimer &Source);

private:

	// Thread synchronization object
	Win32ThreadSynchronizer TS;

	// Class state variables, functions
	double CurrentElapsedTimeCount; // In seconds
	bool IsPaused;
	bool IsStarted;
	void ResetClassStateVariables(void); // Reset (initialize) class state variables

	// Statistics variables, functions
	double MinimumElapsedTime; // Current maximum time statistic
	double MaximumElapsedTime; // Current minimum time statistic
	double AverageElapsedTime; // Current average time statistic
	bool StatisticsAreBlank; // Statistics have default (zeroed out) values
	vector <double> StatisticsVector; // Vector to hold a list of times recorded - This is used to calculate the average elapsed time
	const double& AddTimeToStatistics(const double &StatisticsEntry); // Add a new time entry to the statistics vector

	// These two pure virtual functions must be implemented in the derived classes
	virtual void InitiateTimer(void) = 0; // Sets the start time using the system dependant time counting functions (in 'ticks')
	virtual double HaltTimer(void) = 0;	// Gets the end time and calculates how much time has elapsed in real seconds since the start, returns that value as a double
};



// Win32Timer class
// Uses GetTickCount, a fairly fast but not extremely accurate timer
// Tests for wraparound - I believe that this class is pretty much bulletproof
class Win32Timer : public GenericTimer
{
public:

	Win32Timer(void){;}; // Default constructor
	Win32Timer(const Win32Timer &Source); // Copy constructor
	Win32Timer &operator=(const Win32Timer &Source); // Overloaded assignment operator

private:
	
	DWORD StartingCount; // Variable to keep track of when timing started (in 'ticks')
	void InitiateTimer(void); // Sets the start time using the system dependant time counting functions (in 'ticks')
	double HaltTimer(void); // Gets the end time and calculates how much time has elapsed in real seconds since the start, returns that value as a double
};



// Win32HighResTimer class
// Fast and accurate
// Doesn't test for wraparound
class Win32HighResTimer : public GenericTimer
{
public:

	Win32HighResTimer(void); // Default constructor - this is used to obtain the timer 'tick' frequency (varies among different systems)
	Win32HighResTimer(const Win32HighResTimer &Source); // Copy constructor
	Win32HighResTimer &operator=(const Win32HighResTimer &Source); // Overloaded assignment operator

private:

    LARGE_INTEGER Frequency; // Variable to keep track of the 'tick' frequency
	LARGE_INTEGER StartingCount; // Variable to keep track of when timing started (in 'ticks')
	void InitiateTimer(void); // Sets the start time using the system dependant time counting functions (in 'ticks')
	double HaltTimer(void); // Gets the end time and calculates how much time has elapsed in real seconds since the start, returns that value as a double
};