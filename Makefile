CXX = g++
CXXFLAGS = -Wall -std=c++20 -g

INCDIR = -I./include/ -I./include/ll1 -I./include/slr1
LIBDIR =

SRC = src/main.cpp \
      src/grammar_factory.cpp \
      src/grammar.cpp \
      src/ll1/ll1_parser.cpp \
      src/slr1/slr1_parser.cpp \
      src/slr1/lr0_item.cpp \
      src/symbol_table.cpp

OBJDIR = build/obj
OBJ = $(SRC:.cpp=.o)
OBJ := $(patsubst src/%, $(OBJDIR)/%, $(OBJ))
TARGET = gen

TEST_SRC = src/tests.cpp
TEST_OBJ = $(patsubst src/%, $(OBJDIR)/%, $(TEST_SRC:.cpp=.o))
TEST_TARGET = run_tests
GTEST_LIBS = -lgtest -lgtest_main -lpthread

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $^ -o $@ $(LIBDIR)

$(TEST_TARGET): $(TEST_OBJ) $(filter-out $(OBJDIR)/main.o, $(OBJ))
	$(CXX) $^ -o $@ $(LIBDIR) $(GTEST_LIBS)

$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(TARGET) $(TEST_TARGET)

format:
	@find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

re: fclean all

