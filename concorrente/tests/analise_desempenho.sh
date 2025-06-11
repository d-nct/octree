#!/bin/bash

# ===================================================================
# Script de Análise de Desempenho para a Octree Concorrente
# ===================================================================
# Este script executa o programa de desempenho com várias configurações,
# repete cada teste para obter dados confiáveis e salva os resultados
# em um arquivo CSV para fácil análise e geração de gráficos.
# -------------------------------------------------------------------

# --- CONFIGURAÇÃO DOS TESTES ---

# Edite estes arrays para mudar os parâmetros do teste
THREADS_A_TESTAR=(1 2 4 8)
AMOSTRAS_A_TESTAR=(100000 1000000 5000000 7500000 10000000)
# AMOSTRAS_A_TESTAR=(100 1000 10000)
REPETICOES=10

# Arquivos
EXECUTAVEL="./desempenho"
ARQUIVO_SAIDA="resultados_desempenho.csv"


# --- VERIFICAÇÃO INICIAL ---
if [ ! -f "$EXECUTAVEL" ]; then
    echo "Erro: O binário de teste '$EXECUTAVEL' não foi encontrado."
    echo "Por favor, compile a versão de desempenho primeiro."
    exit 1
fi


# --- EXECUÇÃO DOS TESTES ---
echo "Iniciando análise de desempenho... Isso pode levar vários minutos."
echo "Os resultados serão salvos em '$ARQUIVO_SAIDA'"

# Cria o cabeçalho do arquivo CSV
echo "Threads,Amostras,Repeticao,TempoDeExecucao" > "$ARQUIVO_SAIDA"

# Loop principal sobre cada configuração
for n_threads in "${THREADS_A_TESTAR[@]}"; do
  for n_amostras in "${AMOSTRAS_A_TESTAR[@]}"; do
    echo "-----------------------------------------------------------------"
    echo "Testando com $n_threads Threads e $n_amostras Amostras..."

    for i in $(seq 1 $REPETICOES); do
      printf "  - Repetição %d de %d...\n" "$i" "$REPETICOES"

      # Executa o programa e captura a saída
      SAIDA=$($EXECUTAVEL "$n_threads" "$n_amostras" 2>&1)

      # Extrai a linha de tempo de execução, usando uma pipeline robusta
      # A linha de interesse é "Tempo de execução concorrente: ..." 
      TEMPO=$(echo "$SAIDA" | grep "Tempo de execução concorrente:" | awk -F': ' '{print $2}' | sed 's/[^0-9.]*//g')

      # Verifica se o tempo foi extraído corretamente
      if [ -z "$TEMPO" ]; then
        echo "  AVISO: Não foi possível extrair o tempo para esta execução. Pulando."
        TEMPO="ERRO"
      fi

      # Adiciona a linha de dados ao arquivo CSV
      echo "$n_threads,$n_amostras,$i,$TEMPO" >> "$ARQUIVO_SAIDA"

    done
  done
done

echo "-----------------------------------------------------------------"
echo "Análise de desempenho concluída!"
echo "Resultados salvos com sucesso em '$ARQUIVO_SAIDA'."
