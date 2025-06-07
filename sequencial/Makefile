# ----------------- VARIÁVEIS DE COMPILAÇÃO -----------------
CC = gcc
CFLAGS = -Wall -Wextra -I./src
CFLAGS_DEBUG = $(CFLAGS) -DDEBUG # Liga os LOGS

# ----------------- ARQUIVOS DO PROJETO -----------------
# Código fonte da aplicação principal (testes)
TEST_SRC = ./tests/sequencial.c

# Código fonte dos módulos da Octree
SRCS = ./src/noctree.c ./src/amostra.c

# Nome do arquivo executável de teste que será gerado
TARGET = ./tests/run_tests

# Nome do arquivo executável de debug
TARGET_DEBUG = ./tests/run_tests_debug


# ----------------- REGRAS DE EXECUÇÃO (TARGETS) -----------------

# Regra padrão, executada quando você digita apenas "make"
# Compila a versão normal dos testes.
all: $(TARGET)

# Regra para compilar a versão de debug
debug: $(TARGET_DEBUG)

# Regra para executar os testes padrão. Depende que a regra "all" já tenha sido executada.
test: all
	@echo "--- Executando testes sequenciais ---"
	@$(TARGET)

# Regra para executar os testes de debug. Depende da regra "debug".
test_debug: debug
	@echo "--- Executando testes sequenciais em modo DEBUG ---"
	@$(TARGET_DEBUG)

# Regra para limpar os arquivos gerados
clean:
	@echo "--- Limpando arquivos gerados ---"
	@rm -f $(TARGET) $(TARGET_DEBUG)


# ----------------- REGRAS DE COMPILAÇÃO -----------------

# Regra que ensina o make a criar o executável padrão
$(TARGET): $(TEST_SRC) $(SRCS)
	@echo "--- Compilando a versão padrão ---"
	@$(CC) -o $@ $(CFLAGS) $(TEST_SRC) $(SRCS)
	@echo "Executável padrão '$(TARGET)' criado com sucesso!"

# Regra que ensina o make a criar o executável de debug
$(TARGET_DEBUG): $(TEST_SRC) $(SRCS)
	@echo "--- Compilando a versão de debug ---"
	@$(CC) -o $@ $(CFLAGS_DEBUG) $(TEST_SRC) $(SRCS)
	@echo "Executável de debug '$(TARGET_DEBUG)' criado com sucesso!"


# Declara que certas regras não correspondem a nomes de arquivos
.PHONY: all debug test test_debug clean
