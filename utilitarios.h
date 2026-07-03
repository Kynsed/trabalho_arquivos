#ifndef UTILITARIOS_H
#define UTILITARIOS_H

/*
 * utilitarios.h
 * ---------------
 * Funcoes auxiliares de uso geral: leitura de campos do CSV, leitura de
 * strings entre aspas da entrada padrao e o BinarioNaTela exigido pelo
 * trabalho para conferencia do arquivo gerado.
 */

#include "tipos.h"

/*
 * lerInfo: le um campo do CSV ate encontrar ',', '\n', '\r' ou EOF.
 * Retorna string alocada dinamicamente com o conteudo do campo, ou NULL ao
 * atingir EOF. A liberacao da string e responsabilidade do chamador.
 */
char *lerInfo(FILE *csv);

/*
 * ScanQuoteString: le da entrada padrao um valor que pode estar entre aspas,
 * ser a palavra NULO (resultando em string vazia) ou um token simples.
 * Usado para campos textuais nas funcionalidades de busca/remocao/atualizacao.
 */
void ScanQuoteString(char *str);

/*
 * BinarioNaTela: utilitario fornecido pelo trabalho. Le todo o arquivo
 * binario e imprime um checksum normalizado, usado para conferencia
 * automatica do conteudo gravado.
 */
void BinarioNaTela(char *arquivo);

/*
 * tem_estacao_ativa (auxiliar de [4]): percorre o arquivo procurando algum
 * registro ativo cujo nomeEstacao seja igual a 'nome'. Usado apos uma remocao
 * para decidir se nroEstacoes deve ser decrementado. Preserva a posicao
 * corrente do arquivo.
 */
int tem_estacao_ativa(FILE *input_file, int proxRRN, const char *nome);

/*
 * compE (auxiliar de [13]): comparador para qsort, usado para
 * ordenar registros validos de acordo com codEstacao.
 */
int compE(const void *a, const void *b);

/*
 * compPE (auxiliar de [13]): comparador para qsort, usado para
 * ordenar registros validos de acordo com codProxEstacao.
 */
int compPE(const void *a, const void *b);

#endif /* UTILITARIOS_H */
