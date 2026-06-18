#ifndef JUNCAO_H
#define JUNCAO_H

/*
 * juncao.h
 * ----------
 * Funcionalidades de juncao (autojuncao) do TP2, com condicao de juncao fixa
 * estacao1.codProxEstacao = estacao2.codEstacao:
 *   [11] juncao_loop_aninhado - junção de loop aninhado (forca bruta)
 *   [12] juncao_loop_unico    - junção de loop unico (usa indice arvore-B)
 *
 * Em ambas, para cada registro do arquivo externo sao exibidos os campos
 * codEstacao, nomeEstacao, nomeLinha, codProxEstacao e nomeProxEstacao,
 * sendo este ultimo obtido na juncao (nomeEstacao do registro casado).
 */

#include "tipos.h"

/*
 * [11] juncao_loop_aninhado
 * Realiza a autojuncao por loop aninhado entre arq1 (campo codProxEstacao) e
 * arq2 (campo codEstacao). Nao usa indice. Imprime "Registro inexistente."
 * quando a juncao nao produz nenhum registro e "Falha no processamento do
 * arquivo." em caso de erro de abertura/consistencia.
 */
void juncao_loop_aninhado(char *arq1, char *campo1, char *arq2, char *campo2);

/*
 * [12] juncao_loop_unico
 * Realiza a autojuncao por loop unico: percorre arq1 (loop externo) e, para
 * cada registro, localiza o registro de arq2 que satisfaz a condicao usando o
 * indice arvore-B (arqIndice) definido sobre arq2.
 */
void juncao_loop_unico(char *arq1, char *campo1, char *arq2, char *campo2,
                       char *arqIndice);

#endif /* JUNCAO_H */
