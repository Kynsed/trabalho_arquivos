/*
 * ============================================================================
 *  Trabalho de Organizacao de Arquivos
 * ----------------------------------------------------------------------------
 *  Integrantes:
 *    - <Kelvin Ribeiro Silva>  -  <16302879>
 *    - <Wesley de Brito Sousa>  - <14612350>
 * ----------------------------------------------------------------------------
 *  main.c
 *  Ponto de entrada do programa. Le a opcao da funcionalidade na entrada
 *  padrao e delega para o modulo de funcionalidades correspondente.
 * ============================================================================
 */

#include "funcionalidades.h"
#include "funcionalidades_arvore.h"
#include "juncao.h"

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

        case 9: /* INSERT INTO - insere registro no arquivo de dados e a chave no indice da Arvore-B */
            scanf(" %s %s %d", entradaBin, entradaArvore, &qnt);
            insert_arvore(entradaBin, entradaArvore, qnt);
            break;

        case 10: /* DELETE - busca um registro (usa indice quando ha codEstacao) e o apaga logicamente,
                    também remove a chave do indice */
            scanf(" %s %s %d", entradaBin, entradaArvore, &qnt);
            delete_arvore(entradaBin, entradaArvore, qnt);
            break;

        case 11: /* JOIN por loop aninhado (autojuncao, sem indice) */
        {
            char arq1[100], campo1[100], arq2[100], campo2[100];
            scanf(" %s %s %s %s", arq1, campo1, arq2, campo2);
            juncao_loop_aninhado(arq1, campo1, arq2, campo2);
            break;
        }

        case 12: /* JOIN por loop unico (autojuncao usando indice arvore-B) */
        {
            char arq1[100], campo1[100], arq2[100], campo2[100], idx[100];
            scanf(" %s %s %s %s %s", arq1, campo1, arq2, campo2, idx);
            juncao_loop_unico(arq1, campo1, arq2, campo2, idx);
            break;
        }

        case 13:
            char campo[100], ordenadoBin[100];
            scanf(" %s %s %s", entradaBin, campo, ordenadoBin);
            order_by(entradaBin, campo, ordenadoBin, 1);
            break;

        case 14:
            char arq1[100], campo1[100], arq2[100], campo2[100];
            scanf(" %s %s %s %s", arq1, campo1, arq2, campo2);
            juncao_ordenacao_intercalacao(arq1, campo1, arq2, campo2);
            break;

        default:
            printf("Escolha inválida.\n");
            break;
    }

    return 0;
}
