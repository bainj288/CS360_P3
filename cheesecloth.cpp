#include <iostream>
#include <queue>
#include <pthread.h>
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

bool inputFinished;
bool oddThreadFinished;  
bool fibThreadFinished;
bool collatzThreadFinished;

//Threads
pthread_mutex_t mutexOne = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexTwo = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexThree = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexFour = PTHREAD_MUTEX_INITIALIZER; 

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
        int numberThreads = atoi(argv[1]);

        if(numberThreads != 1)
        {
            cerr << "Milestone 1 only requires one thread" << endl;
            exitNumber = 1;
        }
        else
        {
            pthread_t inputThread;
            pthread_t oddThread;
            pthread_t fibThread;
            pthread_t collatzThread;
            pthread_t outputThread;

            pthread_create(&inputThread, nullptr, threadInput, nullptr);
            pthread_create(&oddThread, nullptr, threadOdd, nullptr);
            pthread_create(&fibThread, nullptr, threadFib, nullptr);
            pthread_create(&collatzThread, nullptr, threadCollatz, nullptr);
            pthread_create(&outputThread, nullptr, threadOutput, nullptr);

            pthread_join(inputThread, nullptr);
            pthread_join(oddThread, nullptr);
            pthread_join(fibThread, nullptr);
            pthread_join(collatzThread, nullptr);
            pthread_join(outputThread, nullptr);
        }
        
    }

    return exitNumber;
}

bool isOdd(unsigned long long number)
{
    return (number % 2 == 1); 
}

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
            pthread_mutex_unlock(&mutexOne);

        }       
    }
    inputFinished = true; 
    return nullptr;
}

void* threadOdd(void* nothing)
{
    bool done = false; 
    while(!done)
    {
        pthread_mutex_lock(&mutexOne);
        if(!queueOne.empty())
        {
            unsigned long long number = queueOne.front(); 
            queueOne.pop();
            pthread_mutex_unlock(&mutexOne);
            if(isOdd(number))
            {
                pthread_mutex_lock(&mutexTwo);
                queueTwo.push(number);
                pthread_mutex_unlock(&mutexTwo);

            }
    
        }
        //Spin lock
        else
        {
            pthread_mutex_unlock(&mutexOne);
            if(inputFinished)
            {
                done = true;
            }

        }   
    }
    oddThreadFinished = true;
    return nullptr;
} 

void* threadFib(void* nothing)
{
    bool done = false;

    while(!done)
    {
        pthread_mutex_lock(&mutexTwo);

        if(!queueTwo.empty())
        {
            unsigned long long number = queueTwo.front();
            queueTwo.pop();
            pthread_mutex_unlock(&mutexTwo);

            if(isFib(number))
            {
                pthread_mutex_lock(&mutexThree);
                queueThree.push(number);
                pthread_mutex_unlock(&mutexThree);
            }
        }
        else
        {
            pthread_mutex_unlock(&mutexTwo);

            if(oddThreadFinished)
            {
                done = true;
            }
        }
    }

    fibThreadFinished = true;
    return nullptr;
}

void* threadCollatz(void* nothing)
{
    bool done = false;

    while(!done)
    {
        pthread_mutex_lock(&mutexThree);

        // if there are numbers to work with after fib thread
        if(!queueThree.empty())
        {
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
                pthread_mutex_unlock(&mutexFour);
            }
        }
        // fib queue is empty
        else
        {
            pthread_mutex_unlock(&mutexThree);

            // if the fib thread is finished and fib queue is empty
            // then this function has no work to do
            if(fibThreadFinished)
            {
                done = true;
            }
        }
    }
    collatzThreadFinished = true;

    return nullptr;
}

void* threadOutput(void* nothing)
{
    bool done = false;

    while(!done)
    {
        pthread_mutex_lock(&mutexFour);

        if(!queueFour.empty())
        {
            unsigned long long number = queueFour.front();
            queueFour.pop();

            pthread_mutex_unlock(&mutexFour);

            // print the number
            cout << number << endl;
        }
        else
        {
            pthread_mutex_unlock(&mutexFour);

            // if the collatz is finished and the queue was empty
            // there is not printing to do and we exit
            if(collatzThreadFinished)
            {
                done = true;
            }
        }
    }

    return nullptr;
}