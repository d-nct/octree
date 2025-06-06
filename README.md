# octree
Implements a procedural and a concurrent version of Octree Algorithm in C.

## Testes

### Compilação dos Testes Sequenciais

Dentro do diretório `./tests/`, executar o comando

```bash
gcc -o run_tests sequencial.c ../src/noctree.c ../src/amostra.c -I../src -Wall
```

para gerar o binário `run_tests`.
