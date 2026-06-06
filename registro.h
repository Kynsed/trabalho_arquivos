#ifndef REGISTRO_H
#define REGISTRO_H

/*
 * registro.h
 * ------------
 * Operacoes sobre o registro de dados (uma estacao): criacao em memoria,
 * leitura do disco, liberacao, impressao e teste de correspondencia com
 * criterios de busca.
 */

#include "tipos.h"

/*
 * criarDados: aloca e inicializa um registro de dados com valores neutros
 * (-1 para inteiros nulos, NULL para strings ausentes). Retorna o ponteiro
 * alocado ou NULL em falha.
 */
struct _dados *criarDados();

/*
 * data_reader: le um registro de tamanho fixo (80 bytes) a partir da posicao
 * atual do arquivo e o desserializa em 'data'. Aloca dinamicamente as strings
 * nomeEstacao/nomeLinha quando presentes.
 * Retorna 1 em sucesso, 0 em EOF/erro de leitura.
 */
int data_reader(Dados *data, FILE *input_file);

/*
 * liberarVetorDados: libera um vetor de ponteiros para Dados (e suas strings
 * internas), usado pela carga do CSV em memoria.
 */
void liberarVetorDados(struct _dados **vetorDados, int tamanho);

/*
 * printDados: imprime os campos de um registro na ordem definida pelo
 * trabalho, exibindo "NULO" para campos ausentes.
 */
void printDados(Dados *data);

/*
 * match_registro: verifica se 'dados' satisfaz todos os criterios em 'vals'.
 * Cada posicao de vals corresponde a um campo; uma string vazia significa
 * "campo nao usado como filtro". Retorna 1 se casar com todos os filtros.
 */
int match_registro(Dados *dados, char vals[8][50]);

#endif /* REGISTRO_H */
