CXX = g++
CXXFLAGS = -Wall -pthread

all: cheesecloth number_gen

cheescloth: cheesecloth.cpp
	$(CXX) $(CXXFLAGS) cheesecloth.cpp -o cheesecloth
	
number_gen: number_gen.cpp
	$(CXX) $(CXXFLAGS) number_gen.cpp -o number_gen
	
clean:
	rm -f cheesecloth number_gen