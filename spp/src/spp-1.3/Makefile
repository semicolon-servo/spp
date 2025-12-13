CXX = g++
CXXFLAGS = -std=c++17 -Wall -I. -g

SRCS = $(shell find servo -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)
TARGET = servocomp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
