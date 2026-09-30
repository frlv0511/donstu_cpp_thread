CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

app: main.cpp threadfuncs.cpp threadfuncs.h
	$(CXX) $(CXXFLAGS) main.cpp threadfuncs.cpp -o app

run: app
	./app

clean:
	rm -f app output.log trace.log

.PHONY: run clean
