# octree
Implements a procedural and a concurrent version of Octree Algorithm in C.

## Testes

Em todos os casos, uma versão explicando o passo a passo sobre o que está acontecendo em cada teste pode ser compilada adicionando a flag `-DDEBUG`. 
O padrão é a flag `DEBUG` desligada, pois, apesar de informativa, ela é bastante verborrágica.

### Compilação dos Testes Sequenciais

Dentro do diretório `./sequencial/tests/`, executar o comando

```bash
gcc -o run_tests sequencial.c ../src/noctree.c ../src/amostra.c -I../src -Wall
```

para gerar o binário `run_tests`.

### Compilação dos Testes Concorrentes 

Dentro do diretório `./concorrente/tests/`, executar o comando

```bash
gcc -o run_tests main.c ../src/noctree.c ../src/amostra.c -I../src -lm -Wall -Wextra
```

para gerar o binário `run_tests`.
