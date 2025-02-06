# 设置编译器
CXX = g++
# 设置编译选项
CXXFLAGS = -std=c++11 -g
# 项目源文件
SRCS = src/main.cpp src/Ext2.cpp src/Shell.cpp
# 项目头文件路径
INCLUDES = -I include
# 目标可执行文件
TARGET = Ext2FileSystem

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)