
CXX = g++
CXXFLAGS = -Wall -std=c++20 -g

INCDIR = -I./
LIBDIR =

SRC = main.cpp \
      grammar_factory.cpp \
      grammar.cpp \
      ll1_parser.cpp \
      slr1_parser.cpp \
      lr0_item.cpp \
      symbol_table.cpp

OBJ = $(SRC:.cpp=.o)

TARGET = gen

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBDIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(OBJ) $(TARGET)

format:
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

re: fclean all

-include $(SRC:.cpp=.d)

