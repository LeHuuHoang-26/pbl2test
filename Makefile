CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures

TARGET = main.exe

# Tự động tìm tất cả file .cpp ở thư mục gốc, Controllers, Models và Data_structures
SOURCES = main.cpp \
          $(wildcard Controllers/*.cpp) \
          $(wildcard Models/*.cpp) \
          $(wildcard Data_structures/*.cpp)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) -mconsole

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q main.o Controllers\*.o Models\*.o Data_structures\*.o $(TARGET) 2>NUL

rebuild: clean all

.PHONY: all clean rebuild