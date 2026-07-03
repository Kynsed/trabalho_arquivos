#ifndef FUNCIONALIDADES_H
#define FUNCIONALIDADES_H

/*
 * funcionalidades.h
 * -------------------
 * Funcionalidades de alto nivel do CRUD, cada uma associada a uma opcao do
 * menu principal:
 *   [1] lerCsv      - carrega um CSV e gera o arquivo binario
 *   [2] select_from - exibe todos os registros validos
 *   [3] busca       - busca registros por criterios
 *   [4] delete_from - remove logicamente registros por criterios
 *   [6] update      - atualiza campos de registros por criterios
 */

#include "tipos.h"

/* [1] Le CSV e gera o arquivo binario correspondente. */
void lerCsv();

/* [2] Le o arquivo binario e exibe todos os registros nao removidos. */
void select_from(char *arquivoEntrada);

/* [3] Executa qntBuscas buscas por criterios sobre o arquivo binario. */
void busca(char *arquivoEntrada, int qntBuscas);

/* [4] Remove logicamente os registros que casam com os criterios informados. */
void delete_from(char *arquivoEntrada);

/* [5] Insere novos registros (reaproveita removidos ou anexa ao fim). */
void inserir(char *arquivoEntrada, int qntInsercoes);

/* [6] Atualiza campos dos registros que casam com os criterios informados. */
void update(char *arquivoEntrada);

void order_by(char *arquivoEntrada, char *campo, char *arquivoOrdenado, int imprimir);

#endif /* FUNCIONALIDADES_H */
