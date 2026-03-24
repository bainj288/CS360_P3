#include <iostream>
#include <queue>
#include <pthread.h>
using namespace std;

bool isOdd(unsigned long long number);
bool isFib(unsigned long long number);
bool reachesForty(unsigned long long number);

//Global Variable
queue<unsigned long long> queueOne;
queue<unsigned long long> queueTwo;
queue<unsigned long long> queueThree;
queue<unsigned long long> queueFour;

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
