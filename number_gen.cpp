#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]){
  if( argc != 3 ){
    cerr << "Usage: number_gen <random_seed> <number_to_generate>" << endl;
    exit(1);
  }
  
  int seed = atoi(argv[1]);
  int total = atoi(argv[2]);

  // generate a bunch of random numbers
  srand(seed);
  for(int i = 0; i < total; i++){
    cout << rand() << endl;
  }
  // print the final 0
  cout << 0 << endl;
}
