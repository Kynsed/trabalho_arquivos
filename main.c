/*
 * ============================================================================
 *  Trabalho de Organizacao de Arquivos
 * ----------------------------------------------------------------------------
 *  Integrantes:
 *    - <Kelvin Ribeiro Silva>  -  <16302879>
 *    - <Nome completo do aluno 2>  -  <nUSP 2>
 * ----------------------------------------------------------------------------
 *  main.c
 *  Ponto de entrada do programa. Le a opcao da funcionalidade na entrada
 *  padrao e delega para o modulo de funcionalidades correspondente.
 *
 *  Opcoes:
 *    [1] lerCsv      - gera o arquivo binario a partir de um CSV
 *    [2] select_from - exibe todos os registros validos
 *    [3] busca       - busca registros por criterios
 *    [4] delete_from - remocao logica de registros
 *    [6] update      - atualizacao de campos de registros
 * ============================================================================
 */

#include "funcionalidades.h"
#include "funcionalidades_arvore.h"

int main(void)
{
    int escolha, qnt;
    char entradaBin[100];    /* nome do arquivo binario de dados */
    char entradaArvore[100]; /* nome do arquivo de indice arvore-B */

    /* Le qual funcionalidade deve ser executada. */
    scanf("%d", &escolha);

    switch (escolha)
    {
        case 1: /* gerar binario a partir do CSV */
            lerCsv();
            break;

        case 2: /* listar todos os registros */
            scanf("%s", entradaBin);
            select_from(entradaBin);
            break;

        case 3: /* buscar por criterios (qnt = numero de buscas) */
            scanf(" %s %d", entradaBin, &qnt);
            busca(entradaBin, qnt);
            break;

        case 4: /* remocao logica */
            scanf(" %s", entradaBin);
            delete_from(entradaBin);
            break;

        case 5: /* insercao de registros */
            scanf(" %s %d", entradaBin, &qnt);
            inserir(entradaBin, qnt);
            break;

        case 6: /* atualizacao de registros */
            scanf(" %s", entradaBin);
            update(entradaBin);
            break;

        case 7: /* cria indice arvore-B sobre codEstacao */
            scanf(" %s %s", entradaBin, entradaArvore);
            criar_indice_arvore(entradaBin, entradaArvore);
            break;

        case 8: /* SELECT ... WHERE (usa indice quando ha codEstacao) */
            scanf(" %s %s %d", entradaBin, entradaArvore, &qnt);
            select_where(entradaBin, entradaArvore, qnt);
            break;

        default:
            printf("Escolha inválida.\n");
            break;
    }

    return 0;
}
