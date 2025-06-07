#!/bin/bash

# --- Script para Teste de Estresse da Octree Concorrente ---

# VERIFICAÇÕES INICIAIS
# -----------------------------------------------------------
# Garante que o número de iterações foi passado como argumento
if [ "$#" -ne 1 ]; then
    echo "Erro: Número de iterações não fornecido."
    echo "Uso: $0 <numero_de_iteracoes>"
    exit 1
fi

# Garante que o argumento é um número inteiro positivo
if ! [[ "$1" =~ ^[1-9][0-9]*$ ]]; then
    echo "Erro: O argumento deve ser um número inteiro positivo."
    exit 1
fi

# Define o caminho para o executável de teste
TEST_EXECUTABLE="./run_tests_conc"

# Verifica se o binário de teste existe antes de continuar
if [ ! -f "$TEST_EXECUTABLE" ]; then
    echo "Erro: O binário de teste não foi encontrado em '$TEST_EXECUTABLE'."
    echo "Por favor, compile o projeto concorrente primeiro."
    exit 1
fi


# INICIALIZAÇÃO E EXECUÇÃO DOS TESTES
# -----------------------------------------------------------
ITERATIONS=$1
TOTAL_PASSOU=0
TOTAL_FALHOU=0

echo "=================================================="
echo "  INICIANDO TESTE DE ESTRESSE DA OCTREE"
echo "  Executando $ITERATIONS iterações..."
echo "=================================================="
echo ""

# Loop principal que executa os testes
for (( i=1; i<=ITERATIONS; i++ )); do
    # Imprime o progresso sem poluir muito a saída
    # printf "Rodando iteração %d de %d...\r" "$i" "$ITERATIONS"

    # Executa o binário e captura sua saída padrão
    OUTPUT=$($TEST_EXECUTABLE)

    # Usa grep e awk para extrair os números das linhas de sumário
    # O ':-0' garante que o valor será 0 se o grep não encontrar a linha (segurança extra)
    PASSOU_NESTA_ITERACAO=$(echo "$OUTPUT" | grep "Testes que Passaram:" | awk '{print $4}')
    PASSOU_NESTA_ITERACAO=${PASSOU_NESTA_ITERACAO:-0}

    FALHOU_NESTA_ITERACAO=$(echo "$OUTPUT" | grep "Testes que Falharam:" | awk '{print $4}')
    FALHOU_NESTA_ITERACAO=${FALHOU_NESTA_ITERACAO:-0}
    
    # Se uma falha for detectada, imprime a saída completa daquela iteração para análise
    if [ "$FALHOU_NESTA_ITERACAO" -ne 0 ]; then
        echo -e "\n\n!!! FALHA DETECTADA NA ITERAÇÃO $i !!!"
        echo "#######################################"
        echo "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
        echo "$OUTPUT"
        echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
        echo "#######################################"
    fi

    # Agrega os resultados nos contadores totais
    (( TOTAL_PASSOU += PASSEI_NESTA_ITERACAO ))
    (( TOTAL_FALHOU += FALHOU_NESTA_ITERACAO ))
done


# SUMÁRIO FINAL
# -----------------------------------------------------------
echo -e "\n\n=================================================="
echo "  SUMÁRIO AGREGADO FINAL"
echo "--------------------------------------------------"
echo "Total de Iterações Executadas: $ITERATIONS"
echo "Total de Testes que Passaram:  $TOTAL_PASSOU"
echo "Total de Testes que Falharam:  $TOTAL_FALHOU"
echo "=================================================="
