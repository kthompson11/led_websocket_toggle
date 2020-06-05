
TARGET := led_ctrl
SRC := $(wildcard *.cpp)
OBJ := $(SRC:%.cpp=%.o)

LIBS := -lgpiodcxx -lgpiod

all : led_ctrl

led_ctrl : $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

%.o : %.c
	$(CXX) -c $(CFLAGS) $(INCDIRS) $< -o $@

clean :
	rm -f $(OBJ)
	rm -f $(TARGET)