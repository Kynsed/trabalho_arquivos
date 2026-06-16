#ifndef ARVOREB_H
#define ARVOREB_H

/*
 * arvoreB.h
 * -----------
 * Tipos e primitivas de manipulacao do indice arvore-B em disco.
 *
 * Caracteristicas (conforme especificacao do TP1):
 *   - Ordem m = 4  => no maximo 3 chaves e 4 descendentes por no (pagina).
 *   - Chave de busca: codEstacao. PR (referencia) = RRN do registro no
 *     arquivo de dados.
 *   - Cabecalho do indice: 17 bytes.
 *   - No (pagina/registro de dados do indice): 53 bytes.
 *   - Insercao com split (sem redistribuicao; nao e arvore-B*).
 */

#include "tipos.h"

/* Ordem da arvore-B e limites derivados. */
#define ORDEM_ARVORE 4                       /* m = 4 */
#define MAX_CHAVES (ORDEM_ARVORE - 1)        /* 3 chaves por no */
#define MAX_FILHOS (ORDEM_ARVORE)            /* 4 descendentes por no */
#define TAM_CABECALHO_ARVORE 17              /* bytes do cabecalho do indice */
#define TAM_NO_ARVORE 53                     /* bytes de um no do indice */

/* Valores de tipoNo. */
#define NO_FOLHA -1          /* no folha (tambem usado quando folha = raiz) */
#define NO_RAIZ 0            /* no raiz com filhos */
#define NO_INTERMEDIARIO 1   /* no interno (nem raiz, nem folha) */

/*
 * CabecalhoArvoreB: primeiro registro (17 bytes) do arquivo de indice.
 * Ordem dos campos segue estritamente a representacao grafica da especificacao:
 * status(1) | noRaiz(4) | topo(4) | proxRRN(4) | nroNos(4).
 */
typedef struct _cabecalhoArvore {
    char status;   /* '0' inconsistente, '1' consistente */
    int noRaiz;    /* RRN da raiz; -1 se a arvore esta vazia */
    int topo;      /* RRN do topo da pilha de nos removidos; -1 se vazia */
    int proxRRN;   /* proximo RRN a ser usado para um novo no */
    int nroNos;    /* numero de nos (paginas) do indice */
} CabecalhoArvoreB;

/*
 * NoArvoreB: um no (pagina) do indice, com 53 bytes em disco.
 * Ordem em disco: removido(1) | proximo(4) | tipoNo(4) | nroChaves(4) |
 *                 C1(4) PR1(4) C2(4) PR2(4) C3(4) PR3(4) |
 *                 P1(4) P2(4) P3(4) P4(4).
 */
typedef struct _noArvore {
    char removido;            /* '0' ativo, '1' logicamente removido */
    int proximo;              /* encadeamento da pilha de nos removidos */
    int tipoNo;               /* -1 folha, 0 raiz, 1 intermediario */
    int nroChaves;            /* numero de chaves preenchidas */
    int C[MAX_CHAVES];        /* chaves de busca (codEstacao) */
    int PR[MAX_CHAVES];       /* referencias ao arquivo de dados (RRN) */
    int P[MAX_FILHOS];        /* ponteiros para subarvores (RRN) ou -1 */
} NoArvoreB;

/* ---- Cabecalho do indice ---- */

/* Inicializa um cabecalho de arvore vazia (status '0', raiz/topo -1). */
void cabecalhoArvore_inicializar(CabecalhoArvoreB *cab);

/*
 * Le os 17 bytes do cabecalho a partir do inicio do arquivo.
 * Retorna 1 se a leitura foi completa e o indice esta consistente; 0 caso
 * contrario.
 */
int cabecalhoArvore_ler(CabecalhoArvoreB *cab, FILE *arq);

/* Grava os 17 bytes do cabecalho no inicio do arquivo (campo a campo). */
void cabecalhoArvore_escrever(CabecalhoArvoreB *cab, FILE *arq);

/* ---- Nos do indice ---- */

/* Inicializa um no vazio: ativo, sem chaves, todos os campos nulos (-1). */
void noArvore_inicializar(NoArvoreB *no);

/* Le o no de RRN 'rrn' (53 bytes) do arquivo. */
void noArvore_ler(NoArvoreB *no, int rrn, FILE *arq);

/* Grava o no 'no' na posicao de RRN 'rrn' (campo a campo, 53 bytes). */
void noArvore_escrever(NoArvoreB *no, int rrn, FILE *arq);

/* ---- Operacoes da arvore ---- */

/*
 * Insere o par (chave, pr) na arvore-B, tratando split e criacao de raiz.
 * Atualiza o cabecalho em memoria (noRaiz, proxRRN, nroNos, topo).
 */
void arvore_inserir(FILE *arq, CabecalhoArvoreB *cab, int chave, int pr);

/*
 * Busca 'chave' na arvore-B. Retorna o PR (RRN no arquivo de dados)
 * correspondente, ou -1 se a chave nao existe no indice.
 */
int arvore_buscar(FILE *arq, CabecalhoArvoreB *cab, int chave);
int arvore_buscar_no(FILE *arq, CabecalhoArvoreB *cab, NoArvoreB *no, int chave);
void delete_chave(FILE *arq, CabecalhoArvoreB *cab, int chave);

#endif /* ARVOREB_H */
