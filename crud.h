#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct cabecalho 
{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroPares;
} Cabecalho;

typedef struct dados 
{
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char* nomeEstacao;
    int tamNomelinha;
    char* nomeLinha;
} Dados;

// Inicialização
void criarCabecalho(Cabecalho* cab);
void criarDados(Dados* dados);

// I/O binário
FILE* open_bin(char* arquivo,char* modo);
int header_reader(Cabecalho* cab, FILE* f);
void header_writer(Cabecalho* cab, FILE* f);
int data_reader(Dados* data, FILE* f);
void data_writer(Dados* data, FILE* f);
void set_status(FILE* f, char status);

// Funcionalidades principais
void lerCsv();
void select_from(char* arquivoEntrada);
void busca(char* arquivoEntrada, int qntBuscas);
void delete_from(char* arquivoEntrada);
void inserir(char* arquivoEntrada, int qntInsercoes);
void update(char* arquivoEntrada);

// Utilitários
int lerInfo(FILE* csv, char buffer[100]);
int novaEstacao(char** nomes, const char* nome, int n);
int match_registro(Dados* dados, char vals[8][50]);
void printDados(Dados* data);
void input_filtro(char campo[50], char valor[50], char vals[8][50]);
int tem_estacao_ativa(FILE* input_file, int proxRRN, const char* nome);

//Funções fornecidas
void BinarioNaTela(char* arquivo);
void ScanQuoteString(char* str);