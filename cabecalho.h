#ifndef CABECALHO_H
#define CABECALHO_H

/*
 * cabecalho.h
 * -------------
 * Operacoes sobre o registro de cabecalho do arquivo binario: criacao em
 * memoria, leitura do disco e gravacao no disco.
 */

#include "tipos.h"

/*
 * criarCabecalho: aloca e inicializa um cabecalho em memoria.
 * O status nasce como '0' (inconsistente): o arquivo so deve ser marcado
 * como consistente ('1') ao final da gravacao bem-sucedida.
 * Retorna ponteiro para o cabecalho ou NULL em falha de alocacao.
 */
Cabecalho *criarCabecalho();

/*
 * header_reader: le os 17 bytes do cabecalho a partir da posicao atual do
 * arquivo (esperada como 0) e preenche 'cab'.
 * Retorna 1 se a leitura foi completa E o arquivo esta consistente
 * (status == '1'); retorna 0 caso contrario.
 */
int header_reader(Cabecalho *cab, FILE *input_file);

/*
 * escreverCabecalho: grava os 17 bytes do cabecalho na posicao atual do
 * arquivo (esperada como 0). Centraliza a ordem de gravacao dos campos para
 * evitar divergencias entre as funcionalidades.
 */
void escreverCabecalho(Cabecalho *cab, FILE *output_file);

#endif /* CABECALHO_H */
