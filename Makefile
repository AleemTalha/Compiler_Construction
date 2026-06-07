CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

SRC = src/Lexer.cpp src/Parser.cpp src/SymbolTable.cpp src/SemanticAnalyzer.cpp src/CodeGenerator.cpp
OBJ = $(SRC:.cpp=.o)

all: compiler

compiler: index.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o compiler index.cpp $(OBJ)

src/%.o: src/%.cpp include/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f compiler src/*.o

run: compiler
	./compiler tests/valid/bool_test.c

test: compiler
	@echo "=== Valid Tests ==="
	./compiler tests/valid/hello.c || true
	@echo ""
	./compiler tests/valid/factorial.c || true
	@echo ""
	./compiler tests/valid/for_loop.c || true
	@echo ""
	./compiler tests/valid/bool_test.c || true
	@echo ""
	@echo "=== Invalid Tests ==="
	./compiler tests/invalid/missing_semicolon.c || true
	@echo ""
	./compiler tests/invalid/type_mismatch.c || true
	@echo ""
	./compiler tests/invalid/undeclared_var.c || true

.PHONY: all clean run test
