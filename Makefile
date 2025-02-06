# Definir el compilador y las banderas
CXX = g++
CXXFLAGS = -Wall -std=c++20 -g

# Definir los directorios de inclusión y las bibliotecas (si es necesario)
INCDIR = -I./
LIBDIR =

# Fuentes y objetos
SRC = main.cpp \
      grammar_factory.cpp \
      grammar.cpp \
      ll1_parser.cpp \
      slr1_parser.cpp \
      lr0_item.cpp \
      symbol_table.cpp

OBJ = $(SRC:.cpp=.o)

# Nombre del ejecutable
TARGET = gen

# Regla para compilar todo
all: $(TARGET)

# Regla para enlazar el ejecutable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBDIR)

# Regla para compilar los archivos .cpp a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -f $(OBJ)

# Regla para eliminar archivos temporales
fclean: clean
	rm -f $(OBJ) $(TARGET)

# Regla para recompilar todo
re: fclean all

# Incluir reglas de depuración (dependencias automáticas)
-include $(SRC:.cpp=.d)

