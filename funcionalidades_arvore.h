#ifndef FUNCIONALIDADES_ARVORE_H
#define FUNCIONALIDADES_ARVORE_H

/*
 * funcionalidades_arvore.h
 * --------------------------
 * Funcionalidades de alto nivel que usam o indice arvore-B (TP1):
 *   [7] criar_indice_arvore - cria o indice arvore-B sobre codEstacao
 *   [8] select_where        - SELECT ... WHERE, usando o indice quando a
 *                             busca envolve a chave primaria (codEstacao)
 */

#include "tipos.h"

/*
 * [7] criar_indice_arvore
 * Cria o arquivo de indice arvore-B (arquivoIndice) para um arquivo de dados
 * ja existente (arquivoDados). Indexa o campo codEstacao de cada registro
 * NAO removido, inserindo as chaves uma a uma. Ao final, chama BinarioNaTela
 * sobre o arquivo de indice.
 */
void criar_indice_arvore(char *arquivoDados, char *arquivoIndice);

/*
 * [8] select_where
 * Recupera os registros do arquivo de dados que satisfazem um criterio de
 * busca. Executa 'n' buscas. Quando codEstacao integra o criterio, usa o
 * indice arvore-B; caso contrario, percorre o arquivo de dados (func. [3]).
 */
void select_where(char *arquivoDados, char *arquivoIndice, int n);
void insert_arvore(char *arquivoDados, char *arquivoIndice, int n);
void delete_arvore(char *arquivoDados, char *arquivoIndice, int n);

#endif /* FUNCIONALIDADES_ARVORE_H */
