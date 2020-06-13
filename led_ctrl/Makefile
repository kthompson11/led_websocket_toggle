
CXXFLAGS := -std=c++11 -Wno-psabi -pthread
debug: CXXFLAGS := $(CXXFLAGS) -g

TARGET := led_ctrl
SRC := $(wildcard *.cpp)
OBJ := $(SRC:%.cpp=%.o)

LIBS := -lgpiodcxx -lgpiod -lpthread

all : led_ctrl

debug : all

led_ctrl : $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

%.o : %.c
	$(CXX) -c $(CXXFLAGS) $(INCDIRS) $< -o $@

clean :
	rm -f $(OBJ)
	rm -f $(TARGET)