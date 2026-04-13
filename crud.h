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
void criar_dados(Dados* dados);
void liberar_dados(Dados* dados);

// I/O
FILE* abrir_arquivo(char* arquivo,char* modo);
int ler_cabecalho(Cabecalho* cab, FILE* f);
void escrever_cabecalho(Cabecalho* cab, FILE* f);
int ler_dados(Dados* data, FILE* f);
void escrever_dados(Dados* data, FILE* f);
void set_status(FILE* f, char status);

// Funcionalidades principais
void ler();
void selecionar(char* arquivoEntrada);
void buscar(char* arquivoEntrada, int qntBuscas);
void deletar(char* arquivoEntrada);
void inserir(char* arquivoEntrada, int qntInsercoes);
void atualizar(char* arquivoEntrada);

// Utilitários
int ler_info(FILE* csv, char buffer[100]);
int tem_estacao_ativa(FILE* input_file, int proxRRN, char* nome);
int nova_estacao(char** nomes, char* nome, int n);
int match_registro(Dados* dados, char vals[8][50]);
void print_dados(Dados* data);
int input_filtro(char campo[50], char valor[50], char vals[8][50]);

//Funções fornecidas
void BinarioNaTela(char* arquivo);
void ScanQuoteString(char* str);
