
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

TEST_SRC = tests.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)
TEST_TARGET = run_tests
GTEST_LIBS = -lgtest -lgtest_main -lpthread
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBDIR)

$(TEST_TARGET): $(TEST_OBJ) $(filter-out main.o, $(OBJ))
	$(CXX) $(filter-out main.o, $(OBJ)) $(TEST_OBJ) -o $(TEST_TARGET) $(LIBDIR) $(GTEST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(OBJ) $(TARGET)

format:
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

re: fclean all

-include $(SRC:.cpp=.d)

