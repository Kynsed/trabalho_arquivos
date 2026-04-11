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

void criarCabecalho(Cabecalho *cab);
void criarDados(Dados *dados);
void lerCsv();
int lerInfo(FILE *csv, char buffer[100]);
int novaEstacao(char **nomes, const char *nome, int n);
void BinarioNaTela(char *arquivo);
void liberarVetorDados(struct _dados **vetorDados, int tamanho);
void busca(char *arquivoEntrada, int qntBuscas);
void select_from(char *arquivoEntrada);
void delete_from(char *arquivoEntrada);
void update(char *arquivoEntrada);
int match_registro(Dados *dados, char vals[8][50]);
void printDados(Dados *data);
void inserir(char *arquivoEntrada, int qntInsercoes);
void ScanQuoteString(char *str);
void input_filtro(char campo[50], char valor[50], char vals[8][50]);