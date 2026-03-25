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


//Global Variable
queue<unsigned long long> queueOne;
queue<unsigned long long> queueTwo;
queue<unsigned long long> queueThree;
queue<unsigned long long> queueFour;

bool inputFinished;
bool oddThreadFinished;  

//Threads
pthread_mutex_t mutexOne = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexTwo = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexThree = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexFour = PTHREAD_MUTEX_INITIALIZER; 

int main(int argc, char* argv[])
{

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
