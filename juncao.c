#include "juncao.h"
#include "cabecalho.h"
#include "registro.h"
#include "arvoreB.h"
#include "funcionalidades.h"


typedef struct _entradaInterna {
    int codEstacao;     /* chave de juncao */
    char *nomeEstacao;  /* usado como nomeProxEstacao na saida */
} EntradaInterna;

/*
 * imprimir_juncao: exibe uma linha de resultado da juncao:
 * codEstacao nomeEstacao nomeLinha codProxEstacao nomeProxEstacao
 * Campos do registro externo vem de 'r1'; nomeProxEstacao e o nome do registro
 */
static void imprimir_juncao(Dados *r1, const char *nomeProxEstacao)
{
    printf("%d %s ", r1->codEstacao, r1->nomeEstacao);

    if (r1->tamNomelinha == 0 || r1->nomeLinha == NULL)
        printf("NULO ");
    else
        printf("%s ", r1->nomeLinha);

    printf("%d %s\n", r1->codProxEstacao, nomeProxEstacao);
}

/*
 * [11] juncao_loop_aninhado
 */
void juncao_loop_aninhado(char *arq1, char *campo1, char *arq2, char *campo2)
{
    /* Nesta funcionalidade apenas codProxEstacao (arq1) e codEstacao (arq2)
     * sao validos como condicao de juncao; os nomes recebidos sao ignorados. */
    (void)campo1;
    (void)campo2;

    FILE *f1 = fopen(arq1, "rb");
    FILE *f2 = fopen(arq2, "rb");
    if (f1 == NULL || f2 == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    Cabecalho cab1, cab2;
    if (!header_reader(&cab1, f1) || !header_reader(&cab2, f2))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    /* Reserva espaco para, no maximo, proxRRN entradas. */
    EntradaInterna *interno = NULL;
    int nInterno = 0;
    if (cab2.proxRRN > 0)
    {
        interno = (EntradaInterna *)malloc(cab2.proxRRN * sizeof(EntradaInterna));
        if (interno == NULL)
        {
            printf("Falha no processamento do arquivo.\n");
            fclose(f1);
            fclose(f2);
            return;
        }
    }

    /* Leitura sequencial de estacao2: guarda apenas registros nao removidos. */
    fseek(f2, TAM_CABECALHO, SEEK_SET);
    for (int i = 0; i < cab2.proxRRN; i++)
    {
        Dados d;
        if (!data_reader(&d, f2))
            break;

        if (d.removido != '1')
        {
            interno[nInterno].codEstacao = d.codEstacao;
            interno[nInterno].nomeEstacao = d.nomeEstacao; /* transfere a posse */
            nInterno++;
            if (d.nomeLinha) free(d.nomeLinha);
        }
        else
        {
            /* Registro removido: descarta tudo. */
            if (d.nomeEstacao) free(d.nomeEstacao);
            if (d.nomeLinha) free(d.nomeLinha);
        }
    }

    int encontrou = 0;
    fseek(f1, TAM_CABECALHO, SEEK_SET);
    for (int i = 0; i < cab1.proxRRN; i++)
    {
        Dados r1;
        if (!data_reader(&r1, f1))
            break;

        if (r1.removido != '1')
        {
            /* Loop interno em memoria. Como codEstacao e chave primaria (unica),
             * ha no maximo um registro casado: interrompe no primeiro encontrado. */
            for (int j = 0; j < nInterno; j++)
            {
                if (r1.codProxEstacao == interno[j].codEstacao)
                {
                    imprimir_juncao(&r1, interno[j].nomeEstacao);
                    encontrou = 1;
                    break;
                }
            }
        }

        if (r1.nomeEstacao) free(r1.nomeEstacao);
        if (r1.nomeLinha) free(r1.nomeLinha);
    }

    if (!encontrou)
        printf("Registro inexistente.\n");

    /* Libera o buffer do arquivo interno. */
    for (int j = 0; j < nInterno; j++)
        free(interno[j].nomeEstacao);
    free(interno);

    fclose(f1);
    fclose(f2);
}

/*
 * [12] juncao_loop_unico */
void juncao_loop_unico(char *arq1, char *campo1, char *arq2, char *campo2,
                       char *arqIndice)
{
    (void)campo1;
    (void)campo2;

    FILE *f1 = fopen(arq1, "rb");
    FILE *f2 = fopen(arq2, "rb");
    FILE *fIdx = fopen(arqIndice, "rb");
    if (f1 == NULL || f2 == NULL || fIdx == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (fIdx) fclose(fIdx);
        return;
    }

    Cabecalho cab1, cab2;
    CabecalhoArvoreB cabArvore;

    /* Os arquivos de dados precisam abrir e estar consistentes. */
    if (!header_reader(&cab1, f1) || !header_reader(&cab2, f2))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        fclose(fIdx);
        return;
    }

    if (!cabecalhoArvore_ler(&cabArvore, fIdx))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        fclose(fIdx);
        return;
    }

    int encontrou = 0;
    fseek(f1, TAM_CABECALHO, SEEK_SET);
    for (int i = 0; i < cab1.proxRRN; i++)
    {
        Dados r1;
        if (!data_reader(&r1, f1))
            break;

        /* Pula removidos; codProxEstacao nulo (-1) nunca casa, evita consulta. */
        if (r1.removido != '1' && r1.codProxEstacao != -1)
        {
            /* Loop unico: usa o indice para localizar o registro de estacao2
             * cujo codEstacao = codProxEstacao do registro externo. */
            int offset = arvore_buscar(fIdx, &cabArvore, r1.codProxEstacao);

            if (offset != -1)
            {
                /* Le, no arquivo de dados, o registro apontado pelo indice. */
                fseek(f2, offset, SEEK_SET);

                Dados r2;
                if (data_reader(&r2, f2))
                {
                    if (r2.removido != '1')
                    {
                        imprimir_juncao(&r1, r2.nomeEstacao);
                        encontrou = 1;
                    }
                    if (r2.nomeEstacao) free(r2.nomeEstacao);
                    if (r2.nomeLinha) free(r2.nomeLinha);
                }
            }
        }

        if (r1.nomeEstacao) free(r1.nomeEstacao);
        if (r1.nomeLinha) free(r1.nomeLinha);
    }

    if (!encontrou)
        printf("Registro inexistente.\n");

    fclose(f1);
    fclose(f2);
    fclose(fIdx);
}

void juncao_ordenacao_intercalacao(char *arq1, char *campo1, char *arq2, char *campo2)
{
    (void)campo1;
    (void)campo2;

    order_by(arq1, "codProxEstacao", arq1, 0);
    order_by(arq2, "codEstacao", arq2, 0);

    FILE *f1 = fopen(arq1, "rb");
    FILE *f2 = fopen(arq2, "rb");
    if (f1 == NULL || f2 == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    Cabecalho cab1, cab2;
    if (!header_reader(&cab1, f1) || !header_reader(&cab2, f2))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    int encontrou = 0, i = 0, j = 0, ibuff = -1, jbuff = -1;
    Dados r1, r2;
    r1.nomeEstacao = r1.nomeLinha = NULL;
    r2.nomeEstacao = r2.nomeLinha = NULL;

    while (i < cab1.proxRRN && j < cab2.proxRRN)
    {
        if (ibuff != i)
        {
            if (r1.nomeEstacao) free(r1.nomeEstacao);
            if (r1.nomeLinha) free(r1.nomeLinha);
            fseek(f1, TAM_CABECALHO + i * TAM_REGISTRO, SEEK_SET);
            if (!data_reader(&r1, f1))
            break;
        }
        if (jbuff != j)
        {
            if (r2.nomeEstacao) free(r2.nomeEstacao);
            if (r2.nomeLinha) free(r2.nomeLinha);
            fseek(f2, TAM_CABECALHO + j * TAM_REGISTRO, SEEK_SET);
            if (!data_reader(&r2, f2))
            break;
        }

        ibuff = i;
        jbuff = j;

        if (r1.codProxEstacao < r2.codEstacao)
        {
            i++;
        }
        else if (r1.codProxEstacao > r2.codEstacao)
        {
            j++;
        }
        else
        {
            imprimir_juncao(&r1, r2.nomeEstacao);
            encontrou = 1;
            i++;
            j++;
        }
    }

    if (r1.nomeEstacao) free(r1.nomeEstacao);
    if (r1.nomeLinha) free(r1.nomeLinha);
    if (r2.nomeEstacao) free(r2.nomeEstacao);
    if (r2.nomeLinha) free(r2.nomeLinha);

    if (!encontrou)
        printf("Registro inexistente.\n");

    fclose(f1);
    fclose(f2);
}