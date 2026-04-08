#include <iostream>
#include <queue>
#include <pthread.h>
#include <list>
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
            pthread_create(&inputThread, nullptr, threadInput, nullptr);

            // create the threads
            for(int i = 0; i < numberThreads; i++)
            {
                pthread_t t1, t2, t3;

                pthread_create(&t1, nullptr, threadOdd, nullptr);
                oddThreads.push_back(t1);

                pthread_create(&t2, nullptr, threadFib, nullptr);
                fibThreads.push_back(t2);

                pthread_create(&t3, nullptr, threadCollatz, nullptr);
                collatzThreads.push_back(t3);
            }

            // start output thread
            pthread_create(&outputThread, nullptr, threadOutput, nullptr);

            // wait for all threads to finish with join
            pthread_join(inputThread, nullptr);

            for(const auto& t : oddThreads)
            {
                pthread_join(t, nullptr);
            }

            for(const auto& t : fibThreads)
            {
                pthread_join(t, nullptr);
            }

            for(const auto& t : collatzThreads)
            {
                pthread_join(t, nullptr);
            }

            pthread_join(outputThread, nullptr);

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
            pthread_mutex_lock(&mutexOne);
            queueOne.push(number);

            // wake one odd thread
            pthread_cond_signal(&conditionOne);
            pthread_mutex_unlock(&mutexOne);

        }       
    }

    // mark input as finished and wake all the other threads
    pthread_mutex_lock(&mutexOne);
    inputFinished = true; 
    pthread_cond_broadcast(&conditionOne);
    pthread_mutex_unlock(&mutexOne);

    return nullptr;
}

// odd thread that filters odd numbers into queueTwo
void* threadOdd(void* nothing)
{
    bool done = false; 
    while(!done)
    {
        pthread_mutex_lock(&mutexOne);

        // wait while there is nothing to do and input is not done
        while(queueOne.empty() && !inputFinished)
        {
            pthread_cond_wait(&conditionOne, &mutexOne);
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

                pthread_mutex_lock(&mutexTwo);
                pthread_cond_broadcast(&conditionTwo);
                pthread_mutex_unlock(&mutexTwo);
            }

            pthread_mutex_unlock(&mutexOne);
            done = true;
        }
        else
        {
            // remove one item from queueOne
            unsigned long long number = queueOne.front(); 
            queueOne.pop();
            pthread_mutex_unlock(&mutexOne);

            if(isOdd(number))
            {
                pthread_mutex_lock(&mutexTwo);
                queueTwo.push(number);

                // wake up one fib thread 
                pthread_cond_signal(&conditionTwo);
                pthread_mutex_unlock(&mutexTwo);

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
        pthread_mutex_lock(&mutexTwo);

        // wait while there is nothing to do and the odd phase may still have work
        while(queueTwo.empty() && !oddThreadFinished)
        {
            pthread_cond_wait(&conditionTwo, &mutexTwo);
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

                pthread_mutex_lock(&mutexThree);
                pthread_cond_broadcast(&conditionThree);
                pthread_mutex_unlock(&mutexThree);
            }

            pthread_mutex_unlock(&mutexTwo);
            done = true;
        }
        else
        {
            // remove one item from queueTwo
            unsigned long long number = queueTwo.front();
            queueTwo.pop();
            pthread_mutex_unlock(&mutexTwo);

            if(isFib(number))
            {
                pthread_mutex_lock(&mutexThree);
                queueThree.push(number);

                // wake a collatz thread because it has work to do
                pthread_cond_signal(&conditionThree);
                pthread_mutex_unlock(&mutexThree);
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
        pthread_mutex_lock(&mutexThree);


        // wait while there is nothing to do and the fib phase is not done
        while(queueThree.empty() && !fibThreadFinished)
        {
            pthread_cond_wait(&conditionThree, &mutexThree);
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

                pthread_mutex_lock(&mutexFour);
                pthread_cond_broadcast(&conditionFour);
                pthread_mutex_unlock(&mutexFour);
            }

            pthread_mutex_unlock(&mutexThree);
            done = true;
        }
        else
        {
            // remove one item from queueThree
            unsigned long long number = queueThree.front();
            queueThree.pop();

            pthread_mutex_unlock(&mutexThree);

            // check whether number reaches 40
            // this uses the Collatz procedure
            if(reachesForty(number))
            {
                // if it passes then we want to move it to the output queue
                pthread_mutex_lock(&mutexFour);
                queueFour.push(number);

                // wake output thread because there is something to print
                pthread_cond_signal(&conditionFour);
                pthread_mutex_unlock(&mutexFour);
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
        pthread_mutex_lock(&mutexFour);

        // wait while there is nothing to print and collatz phase is not done
        while(queueFour.empty() && !collatzThreadFinished)
        {
            pthread_cond_wait(&conditionFour, &mutexFour);
        }

        // if queueFour is empty and collatz phase is done then output thread can finish
        if(queueFour.empty() && collatzThreadFinished)
        {
            pthread_mutex_unlock(&mutexFour);
            done = true;
        }
        else
        {
            // remove one number and print it
            unsigned long long number = queueFour.front();
            queueFour.pop();

            pthread_mutex_unlock(&mutexFour);

            // print the number
            cout << number << endl;
        }
        
    }

    return nullptr;
}