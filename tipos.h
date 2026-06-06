#ifndef TIPOS_H
#define TIPOS_H

/*
 * tipos.h
 * ----------
 * Centraliza as definicoes de tipos usadas por todos os modulos do programa
 * e os cabecalhos padrao da biblioteca C. Reunir os tipos aqui permite que
 * cada modulo (.c) inclua apenas este arquivo para enxergar Cabecalho e Dados,
 * evitando duplicacao de definicoes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Tamanho fixo, em bytes, do registro de dados gravado em disco. */
#define TAM_REGISTRO 80
/* Tamanho fixo, em bytes, do registro de cabecalho gravado em disco. */
#define TAM_CABECALHO 17
/* Bytes ocupados pelos campos de tamanho fixo do registro
 * (removido + 7 inteiros + tamNomeEstacao + tamNomelinha = 1 + 9*4 = 37). */
#define TAM_FIXO_REGISTRO 37

/*
 * Cabecalho: primeiro registro do arquivo binario. Guarda o estado global
 * do arquivo e os ponteiros de controle da pilha de registros removidos.
 */
typedef struct _cabecalho {
    char status;      /* '0' = arquivo inconsistente, '1' = consistente */
    int topo;         /* RRN do topo da pilha de registros removidos (-1 se vazia) */
    int proxRRN;      /* proximo RRN a ser inserido (total de registros gravados) */
    int nroEstacoes;  /* numero de estacoes distintas cadastradas */
    int nroPares;     /* numero de pares de estacoes cadastrados */
} Cabecalho;

/*
 * Dados: registro de uma estacao. Campos de tamanho variavel (nomeEstacao,
 * nomeLinha) sao acompanhados de seus tamanhos para permitir leitura em
 * registro de tamanho fixo.
 */
typedef struct _dados {
    char removido;          /* '0' = ativo, '1' = logicamente removido */
    int proximo;            /* encadeamento da pilha de removidos */
    int codEstacao;         /* chave primaria da estacao */
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;     /* tamanho de nomeEstacao */
    char *nomeEstacao;
    int tamNomelinha;       /* tamanho de nomeLinha */
    char *nomeLinha;
} Dados;

#endif /* TIPOS_H */
