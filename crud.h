#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct _cabecalho {
    char status; // 0 ou 1, indicando se o arquivo está consistente ou inconsistente
    int topo; // byte offset do topo da pilha de registros removidos
    int proxRRN; // próximo RRN a ser inserido
    int nroEstacoes; // número de estações cadastradas
    int nroPares; // número de pares de estações cadastrados
}Cabecalho;

typedef struct _dados {
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao;
    int tamNomelinha;
    char *nomeLinha;
}Dados;

struct _cabecalho *criarCabecalho();
struct _dados *criarDados();
void lerCsv();
char *lerInfo(FILE *csv);
int novaEstacao(struct _dados **vetorDados, int n, const char *nome);
void BinarioNaTela(char *arquivo);
void liberarVetorDados(struct _dados **vetorDados, int tamanho);
void busca(char *arquivoEntrada, int qntBuscas);
void select_from(char *arquivoEntrada);
void printDados(Dados *data);