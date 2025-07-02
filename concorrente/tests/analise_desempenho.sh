#!/bin/bash

# ===================================================================
# Script de Análise de Desempenho para a Octree Concorrente
# ===================================================================
# Este script executa o programa de desempenho com várias configurações,
# repete cada teste para obter dados confiáveis e salva os resultados
# em um arquivo CSV para fácil análise e geração de gráficos.
# -------------------------------------------------------------------

# --- VERIFICAÇÃO INICIAL ---
if [ -z "$1" ] || ([ "$1" != "busca" ] && [ "$1" != "criacao" ]); then
  echo "Argumentos inválidos! Uso: $0 [busca|criacao]"
  exit 1
fi

TIPO_TESTE=$1

# --- CONFIGURAÇÃO DOS TESTES ---

# Parâmetros para o teste de CRIAÇÃO
if [ "$TIPO_TESTE" == "criacao" ]; then
  THREADS_A_TESTAR=(1 2 4 8)
  AMOSTRAS_A_TESTAR=(100000 1000000 5000000 7500000 10000000)
  REPETICOES=10
  EXECUTAVEL="./desempenho_criacao"
  ARQUIVO_SAIDA="resultados_desempenho_criacao.csv"
  COLUNAS_CSV="Threads,Amostras,Repeticao,TempoDeExecucao"
  LINHA_GREP="Tempo de execução concorrente:"

# Parâmetros para o teste de BUSCA
else # busca
  THREADS_ESCRITA_A_TESTAR=(4) # ótima
  THREADS_LEITURA_A_TESTAR=(1 2 4 8)
  AMOSTRAS_A_TESTAR=(1000000 5000000)
  BUSCAS_A_TESTAR=(1000 10000 50000 100000 500000)
  REPETICOES=5
  EXECUTAVEL="./desempenho_busca"
  ARQUIVO_SAIDA="resultados_desempenho_busca.csv"
  COLUNAS_CSV="ThreadsEscrita,ThreadsLeitura,Amostras,Buscas,Repeticao,TempoDeBusca"
  LINHA_GREP="Tempo de buscas na octree:"
fi


# --- COMPILAÇÃO ---
echo "Compilando o binário de teste '$EXECUTAVEL'..."
gcc -o $EXECUTAVEL "${EXECUTAVEL}.c" ../src/noctree.c ../src/amostra.c -I ../src/ -Wall -Wextra -lm
if [ $? -ne 0 ]; then
    echo "Erro de compilação. Abortando."
    exit 1
fi
echo "Compilação bem-sucedida."


# --- EXECUÇÃO DOS TESTES ---
echo "Iniciando análise de desempenho para '$TIPO_TESTE'..."
echo "Os resultados serão salvos em '$ARQUIVO_SAIDA'"

# Cria o cabeçalho do arquivo CSV
echo "$COLUNAS_CSV" > "$ARQUIVO_SAIDA"

# Loop principal sobre cada configuração
if [ "$TIPO_TESTE" == "criacao" ]; then
  for n_threads in "${THREADS_A_TESTAR[@]}"; do
    for n_amostras in "${AMOSTRAS_A_TESTAR[@]}"; do
      echo "-----------------------------------------------------------------"
      echo "Testando com $n_threads Threads e $n_amostras Amostras..."

      for i in $(seq 1 $REPETICOES); do
        printf "  - Repetição %d de %d...\n" "$i" "$REPETICOES"
        SAIDA=$($EXECUTAVEL "$n_threads" "$n_amostras" 2>&1)
        TEMPO=$(echo "$SAIDA" | grep "$LINHA_GREP" | awk -F': ' '{print $2}' | sed 's/[^0-9.]*//g')
        if [ -z "$TEMPO" ]; then
          echo "  AVISO: Não foi possível extrair o tempo para esta execução. Pulando."
          TEMPO="ERRO"
        fi
        echo "$n_threads,$n_amostras,$i,$TEMPO" >> "$ARQUIVO_SAIDA"
      done
    done
  done
else # busca
  for n_threads_esc in "${THREADS_ESCRITA_A_TESTAR[@]}"; do
    for n_threads_leit in "${THREADS_LEITURA_A_TESTAR[@]}"; do
      for n_amostras in "${AMOSTRAS_A_TESTAR[@]}"; do
        for n_buscas in "${BUSCAS_A_TESTAR[@]}"; do
          echo "-----------------------------------------------------------------"
          echo "Testando com $n_threads_esc (E), $n_threads_leit (L), $n_amostras amostras, $n_buscas buscas..."

          for i in $(seq 1 $REPETICOES); do
            printf "  - Repetição %d de %d...\n" "$i" "$REPETICOES"
            SAIDA=$($EXECUTAVEL "$n_threads_esc" "$n_threads_leit" "$n_amostras" "$n_buscas" 2>&1)
            TEMPO=$(echo "$SAIDA" | grep "$LINHA_GREP" | awk -F': ' '{print $2}' | sed 's/[^0-9.]*//g')
            if [ -z "$TEMPO" ]; then
              echo "  AVISO: Não foi possível extrair o tempo para esta execução. Pulando."
              TEMPO="ERRO"
            fi
            echo "$n_threads_esc,$n_threads_leit,$n_amostras,$n_buscas,$i,$TEMPO" >> "$ARQUIVO_SAIDA"
          done
        done
      done
    done
  done
fi

echo "-----------------------------------------------------------------"
echo "Análise de desempenho concluída!"
echo "Resultados salvos com sucesso em '$ARQUIVO_SAIDA'."
