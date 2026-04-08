#include <iostream>
#include <queue>
#include <pthread.h>
#include <list>
#include <cstring>
using namespace std;

//Prototypes
bool isOdd(unsigned long long number);
bool isFib(unsigned long long number);
bool reachesForty(unsigned long long number);
void* threadInput(void* nothing);
void* threadOdd(void* nothing);
void* threadFib(void* nothing);
void* threadCollatz(void* nothing);
void* threadOutput(void* nothing);
void handleError(int returnCode, const char* errorMessage);


//Global Variable
queue<unsigned long long> queueOne;
queue<unsigned long long> queueTwo;
queue<unsigned long long> queueThree;
queue<unsigned long long> queueFour;

// flags to indicate phase completion 
bool inputFinished;
bool oddThreadFinished;  
bool fibThreadFinished;
bool collatzThreadFinished;

// count how many threads finish in each phase
int oddThreadsDone = 0;
int fibThreadsDone = 0;
int collatzThreadsDone = 0;

// count of threads per work queue
int numberThreads = 0;

//Threads
pthread_mutex_t mutexOne = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexTwo = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexThree = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexFour = PTHREAD_MUTEX_INITIALIZER; 

// condition variables
pthread_cond_t conditionOne = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionTwo = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionThree = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionFour = PTHREAD_COND_INITIALIZER;

// helper for pthread errors
void handleError(int returnCode, const char* message)
{
    if(returnCode != 0)
    {
        char buffer[256];
        strerror_r(returnCode, buffer, sizeof(buffer));
        cerr << message << " : " << buffer << endl;
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int exitNumber = 0;

    // program should take one command line arg - number of threads per work queue
    if(argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <number_of_threads>" << endl;
        exitNumber = 1;
    }
    else
    {
        numberThreads = atoi(argv[1]);

        if(numberThreads <= 0)
        {
            cerr << "Number of threads must be greater than 0" << endl;
            exitNumber = 1;
        }
        else
        {
            pthread_t inputThread;
            pthread_t outputThread;

            // lists to store the worker threads
            list<pthread_t> oddThreads;
            list<pthread_t> fibThreads;
            list<pthread_t> collatzThreads;

            // start the input thread
            handleError(pthread_create(&inputThread, nullptr, threadInput, nullptr), "Error creating input thread");

            // create the threads
            for(int i = 0; i < numberThreads; i++)
            {
                pthread_t t1, t2, t3;

                handleError(pthread_create(&t1, nullptr, threadOdd, nullptr), "Error creating odd thread");
                oddThreads.push_back(t1);

                handleError(pthread_create(&t2, nullptr, threadFib, nullptr), "Error creating fib thread");
                fibThreads.push_back(t2);

                handleError(pthread_create(&t3, nullptr, threadCollatz, nullptr), "Error creating collatz thread");
                collatzThreads.push_back(t3);
            }

            // start output thread
            handleError(pthread_create(&outputThread, nullptr, threadOutput, nullptr), "Error creating output thread");

            // wait for all threads to finish with join
            handleError(pthread_join(inputThread, nullptr), "Error joining input thread");

            for(const auto& t : oddThreads)
            {
                handleError(pthread_join(t, nullptr), "Error joining odd thread");
            }

            for(const auto& t : fibThreads)
            {
                handleError(pthread_join(t, nullptr), "Error joining fib threads");
            }

            for(const auto& t : collatzThreads)
            {
                handleError(pthread_join(t, nullptr), "Error joining collatz threads");
            }

            handleError(pthread_join(outputThread, nullptr), "Error joining output thread");

        }
        
    }

    return exitNumber;
}

// helper to check if number is odd
bool isOdd(unsigned long long number)
{
    return (number % 2 == 1); 
}

// helper to check if number is Fibonacci
bool isFib(unsigned long long number)
{
    bool result = false;
    if(number == 1)
    {
        result = true;
    }

    else
    {
        unsigned long long fibOne = 1;
        unsigned long long fibTwo = 1;

        // generate fibonacci numbers until >= number
        while(fibTwo < number)
        {
            unsigned long long fibNext = fibOne + fibTwo; 
            fibOne = fibTwo; 
            fibTwo = fibNext;
        }
        if(fibTwo == number)
        {
            result = true;
        }
    }
    return result; 
}

// collatz checker if it reaches 40
bool reachesForty(unsigned long long number)
{
    bool result = false; 

    while(number != 1 && number != 40)
    {
        if(number % 2 == 0)
        {
            number /= 2;
        }

        else
        {
            number = (3 * number) + 1;
        }
    }

    if(number == 40 )
    {
        result = true; 
    }

    return result; 
}

// input thread that reads numbers and pushes to queueOne
void* threadInput(void* nothing)
{
    unsigned long long number; 
    bool done = false;
    while((cin >> number) && !done)
    {
        if(number == 0)
        {
            done = true;
        } 
        else
        {
            handleError(pthread_mutex_lock(&mutexOne), "Error locking mutexOne");
            queueOne.push(number);

            // wake one odd thread
            handleError(pthread_cond_signal(&conditionOne), "Error signaling conditionOne");
            handleError(pthread_mutex_unlock(&mutexOne), "Error unlocking mutexOne");

        }       
    }

    // mark input as finished and wake all the other threads
    handleError(pthread_mutex_lock(&mutexOne), "Error locking mutexOne");
    inputFinished = true; 
    handleError(pthread_cond_broadcast(&conditionOne), "Error broadcasting conditionOne");
    handleError(pthread_mutex_unlock(&mutexOne), "Error unlocking mutexOne");

    return nullptr;
}

// odd thread that filters odd numbers into queueTwo
void* threadOdd(void* nothing)
{
    bool done = false; 
    while(!done)
    {
        handleError(pthread_mutex_lock(&mutexOne), "Error locking mutexOne");

        // wait while there is nothing to do and input is not done
        while(queueOne.empty() && !inputFinished)
        {
            handleError(pthread_cond_wait(&conditionOne, &mutexOne), "Error waiting on conditionOne");
        }

        // if queue is empty and input thread is done, we can proceed
        if(queueOne.empty() && inputFinished)
        {
            oddThreadsDone++;

            // if this is the last thread that needs to finish
            // mark phase as done and wake all the fib threads
            if(oddThreadsDone == numberThreads)
            {

                oddThreadFinished = true;

                handleError(pthread_mutex_lock(&mutexTwo), "Error locking mutexTwo");
                handleError(pthread_cond_broadcast(&conditionTwo), "Error broadcasting conditionTwo");
                handleError(pthread_mutex_unlock(&mutexTwo), "Error unlocking mutexTwo");
            }

            pthread_mutex_unlock(&mutexOne);
            done = true;
        }
        else
        {
            // remove one item from queueOne
            unsigned long long number = queueOne.front(); 
            queueOne.pop();
            handleError(pthread_mutex_unlock(&mutexOne), "Error unlocking mutexOne");

            if(isOdd(number))
            {
                handleError(pthread_mutex_lock(&mutexTwo), "Error locking mutexTwo");
                queueTwo.push(number);

                // wake up one fib thread 
                handleError(pthread_cond_signal(&conditionTwo), "Error signaling conditionTwo");
                handleError(pthread_mutex_unlock(&mutexTwo), "Error unlocking mutexTwo");

            }
        }

    }

    return nullptr;
} 

// fib thread that filters fib numbers into queueThree
void* threadFib(void* nothing)
{
    bool done = false;

    while(!done)
    {
        handleError(pthread_mutex_lock(&mutexTwo), "Error locking mutexTwo");

        // wait while there is nothing to do and the odd phase may still have work
        while(queueTwo.empty() && !oddThreadFinished)
        {
            handleError(pthread_cond_wait(&conditionTwo, &mutexTwo), "Error waiting on conditionTwo");
        }

        // if queueTwo is empty and the odd phase is finished then fib thread can finish
        if(queueTwo.empty() && oddThreadFinished)
        {
            fibThreadsDone++;

            // if this is the last fib thread then fib phase is done
            // wake all of the collatz threads
            if(fibThreadsDone == numberThreads)
            {
                fibThreadFinished = true;

                handleError(pthread_mutex_lock(&mutexThree), "Error locking mutexThree");
                handleError(pthread_cond_broadcast(&conditionThree), "Error broadcasting conditionThree");
                handleError(pthread_mutex_unlock(&mutexThree), "Error unlocking mutexThree");
            }

            handleError(pthread_mutex_unlock(&mutexTwo), "Error unlocking mutexTwo");
            done = true;
        }
        else
        {
            // remove one item from queueTwo
            unsigned long long number = queueTwo.front();
            queueTwo.pop();
            handleError(pthread_mutex_unlock(&mutexTwo), "Error unlocking mutexTwo");

            if(isFib(number))
            {
                handleError(pthread_mutex_lock(&mutexThree), "Error locking mutexThree");
                queueThree.push(number);

                // wake a collatz thread because it has work to do
                handleError(pthread_cond_signal(&conditionThree), "Error signaling conditionThree");
                handleError(pthread_mutex_unlock(&mutexThree), "Error unlocking mutexThree");
            }
        }
        
    }
    return nullptr;
}

// collatz thread to filter numbers that reach 40 into queueFour
void* threadCollatz(void* nothing)
{
    bool done = false;

    while(!done)
    {
        handleError(pthread_mutex_lock(&mutexThree), "Error locking mutexThree");


        // wait while there is nothing to do and the fib phase is not done
        while(queueThree.empty() && !fibThreadFinished)
        {
            handleError(pthread_cond_wait(&conditionThree, &mutexThree), "Error waiting on conditionThree");
        }

        // if queueThree is empty and fib phase is done then this thread can finish
        if(queueThree.empty() && fibThreadFinished)
        {
            collatzThreadsDone++;

            // if this is the last collatz thread mark phase as done
            // wake output thread
            if(collatzThreadsDone == numberThreads)
            {
                collatzThreadFinished = true; 

                handleError(pthread_mutex_lock(&mutexFour), "Error locking mutexFour");
                handleError(pthread_cond_broadcast(&conditionFour), "Error broadcasting conditionFour");
                handleError(pthread_mutex_unlock(&mutexFour), "Error unlocking mutexFour");
            }

            handleError(pthread_mutex_unlock(&mutexThree), "Error unlocking mutexThree");
            done = true;
        }
        else
        {
            // remove one item from queueThree
            unsigned long long number = queueThree.front();
            queueThree.pop();

            handleError(pthread_mutex_unlock(&mutexThree), "Error unlocking mutexThree");

            // check whether number reaches 40
            // this uses the Collatz procedure
            if(reachesForty(number))
            {
                // if it passes then we want to move it to the output queue
                handleError(pthread_mutex_lock(&mutexFour), "Error locking mutexFour");
                queueFour.push(number);

                // wake output thread because there is something to print
                handleError(pthread_cond_signal(&conditionFour), "Error signaling conditionFour");
                handleError(pthread_mutex_unlock(&mutexFour), "Error unlocking mutexFour");
            }
        }
        
    }

    return nullptr;
}

// output thread to print the final results 
void* threadOutput(void* nothing)
{
    bool done = false;

    while(!done)
    {
        handleError(pthread_mutex_lock(&mutexFour), "Error locking mutexFour");

        // wait while there is nothing to print and collatz phase is not done
        while(queueFour.empty() && !collatzThreadFinished)
        {
            handleError(pthread_cond_wait(&conditionFour, &mutexFour), "Error waiting on conditionFour");
        }

        // if queueFour is empty and collatz phase is done then output thread can finish
        if(queueFour.empty() && collatzThreadFinished)
        {
            handleError(pthread_mutex_unlock(&mutexFour), "Error unlocking mutexFour");
            done = true;
        }
        else
        {
            // remove one number and print it
            unsigned long long number = queueFour.front();
            queueFour.pop();

            handleError(pthread_mutex_unlock(&mutexFour), "Error unlocking mutexFour");

            // print the number
            cout << number << endl;
        }
        
    }

    return nullptr;
}