CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -I./Models -I./Controllers -I./Data_structures

TARGET = main.exe

# Tự động quét toàn bộ file mã nguồn .cpp
SOURCES = main.cpp \
          $(wildcard Controllers/*.cpp) \
          $(wildcard Models/*.cpp) \
          $(wildcard Data_structures/*.cpp)

all: $(TARGET)

# Biên dịch trực tiếp từ .cpp sang .exe, không sinh ra file trung gian .o
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) -mconsole

clean:
	del /Q $(TARGET) 2>NUL

rebuild: clean all

.PHONY: all clean rebuild