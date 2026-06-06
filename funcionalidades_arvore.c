#include "funcionalidades_arvore.h"
#include "arvoreB.h"
#include "cabecalho.h"
#include "registro.h"
#include "utilitarios.h"

/*
 * ler_criterios (auxiliar de [8]): le 'm' pares (nomeCampo valorCampo) da
 * entrada padrao e os armazena em 'vals', indexados na ordem dos campos do
 * registro de dados:
 *   0=codEstacao 1=nomeEstacao 2=codLinha 3=nomeLinha
 *   4=codProxEstacao 5=distProxEstacao 6=codLinhaIntegra 7=codEstIntegra
 * Campos string sao lidos com ScanQuoteString (aspas duplas); o marcador NULO
 * e normalizado para que match_registro o reconheca.
 */
static void ler_criterios(int m, char vals[8][50])
{
    /* Zera todos os filtros (string vazia = campo nao usado na busca). */
    for (int i = 0; i < 8; i++)
        vals[i][0] = '\0';

    char campo[50], valor[100];

    for (int k = 0; k < m; k++)
    {
        scanf(" %s", campo);

        /* Campos de tamanho variavel usam ScanQuoteString. */
        if (!strcmp(campo, "nomeEstacao") || !strcmp(campo, "nomeLinha"))
        {
            ScanQuoteString(valor);
            /* ScanQuoteString devolve "" para NULO; representa o nulo
             * explicitamente para que match_registro trate o caso. */
            if (valor[0] == '\0')
                strcpy(valor, "NULO");
        }
        else
        {
            scanf(" %s", valor);
            /* Campos inteiros nulos sao gravados como -1 no arquivo. */
            if (strcmp(valor, "NULO") == 0)
                strcpy(valor, "-1");
        }

        if (!strcmp(campo, "codEstacao"))
            strcpy(vals[0], valor);
        else if (!strcmp(campo, "nomeEstacao"))
            strcpy(vals[1], valor);
        else if (!strcmp(campo, "codLinha"))
            strcpy(vals[2], valor);
        else if (!strcmp(campo, "nomeLinha"))
            strcpy(vals[3], valor);
        else if (!strcmp(campo, "codProxEstacao"))
            strcpy(vals[4], valor);
        else if (!strcmp(campo, "distProxEstacao"))
            strcpy(vals[5], valor);
        else if (!strcmp(campo, "codLinhaIntegra"))
            strcpy(vals[6], valor);
        else if (!strcmp(campo, "codEstIntegra"))
            strcpy(vals[7], valor);
    }
}

/*
 * busca_por_indice (auxiliar de [8]): trata uma busca que inclui codEstacao.
 * Usa o indice arvore-B para localizar o unico registro candidato (chave
 * primaria), valida os demais criterios e o exibe. Imprime "Registro
 * inexistente." quando nao ha registro correspondente.
 */
static void busca_por_indice(FILE *dados, CabecalhoArvoreB *cabArvore,
                             FILE *indice, char vals[8][50])
{
    int chave = atoi(vals[0]);

    /* Consulta o indice: obtem o byte offset do registro no arquivo de dados. */
    int prByte = arvore_buscar(indice, cabArvore, chave);
    if (prByte == -1)
    {
        printf("Registro inexistente.\n");
        return;
    }

    /* PR ja e o offset em bytes: posiciona diretamente no registro. */
    fseek(dados, prByte, SEEK_SET);

    Dados data;
    if (!data_reader(&data, dados))
    {
        printf("Registro inexistente.\n");
        return;
    }

    /* Valida remocao logica e os criterios adicionais (alem da chave). */
    if (data.removido == '1' || !match_registro(&data, vals))
    {
        printf("Registro inexistente.\n");
    }
    else
    {
        printDados(&data);
        printf("\n");
    }

    if (data.nomeEstacao) free(data.nomeEstacao);
    if (data.nomeLinha) free(data.nomeLinha);
}

/*
 * busca_sequencial (auxiliar de [8]): trata buscas que NAO usam codEstacao,
 * conforme a especificacao da funcionalidade [3]. Percorre o arquivo de dados,
 * pula registros removidos e exibe os que satisfazem todos os criterios.
 */
static void busca_sequencial(FILE *dados, int proxRRN, char vals[8][50])
{
    /* Posiciona no primeiro registro (logo apos o cabecalho). */
    fseek(dados, TAM_CABECALHO, SEEK_SET);

    int encontrou = 0;

    for (int i = 0; i < proxRRN; i++)
    {
        Dados data;
        if (!data_reader(&data, dados))
            break;

        /* Pula registros logicamente removidos. */
        if (data.removido == '1')
        {
            if (data.nomeEstacao) free(data.nomeEstacao);
            if (data.nomeLinha) free(data.nomeLinha);
            continue;
        }

        if (match_registro(&data, vals))
        {
            printDados(&data);
            printf("\n");
            encontrou = 1;
        }

        if (data.nomeEstacao) free(data.nomeEstacao);
        if (data.nomeLinha) free(data.nomeLinha);
    }

    if (!encontrou)
        printf("Registro inexistente.\n");
}

/*
 * [7] criar_indice_arvore
 * Le os RRNs do arquivo de dados e insere, um a um, a chave codEstacao dos
 * registros nao removidos no indice arvore-B.
 */
void criar_indice_arvore(char *arquivoDados, char *arquivoIndice)
{
    FILE *dados;

    /* Abre e valida o arquivo de dados (deve estar consistente). */
    if (arquivoDados == NULL || !(dados = fopen(arquivoDados, "rb")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabDados;
    if (!header_reader(&cabDados, dados))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(dados);
        return;
    }

    /* Cria o arquivo de indice (leitura+escrita binaria). */
    FILE *indice = fopen(arquivoIndice, "wb+");
    if (indice == NULL)
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(dados);
        return;
    }

    /* Cabecalho do indice de uma arvore vazia, marcado como inconsistente. */
    CabecalhoArvoreB cabArvore;
    cabecalhoArvore_inicializar(&cabArvore);
    cabecalhoArvore_escrever(&cabArvore, indice);

    /* Percorre o arquivo de dados de forma sequencial (sem fseek por registro).
     * Le apenas o necessario para indexar: o byte 'removido' e a chave
     * codEstacao, evitando alocacao das strings de cada registro (economia de
     * RAM e de chamadas de I/O). */
    fseek(dados, TAM_CABECALHO, SEEK_SET);

    unsigned char rec[TAM_REGISTRO]; /* buffer de pilha; nenhum heap por registro */

    for (int rrn = 0; rrn < cabDados.proxRRN; rrn++)
    {
        /* Uma unica leitura sequencial avanca o ponteiro ao proximo registro. */
        if (fread(rec, TAM_REGISTRO, 1, dados) != 1)
            break;

        /* Registros removidos nao sao indexados (byte 0 == '1'). */
        if (rec[0] == '1')
            continue;

        /* codEstacao fica logo apos removido(1) e proximo(4): offset 5. */
        int codEstacao;
        memcpy(&codEstacao, rec + 1 + sizeof(int), sizeof(int));

        /* PR e o byte offset do registro no arquivo de dados. */
        int pr = (int)(TAM_CABECALHO + (long)TAM_REGISTRO * rrn);
        arvore_inserir(indice, &cabArvore, codEstacao, pr);
    }

    /* Conclui: marca o indice como consistente e regrava o cabecalho. */
    cabArvore.status = '1';
    cabecalhoArvore_escrever(&cabArvore, indice);

    fclose(dados);
    fclose(indice);

    BinarioNaTela(arquivoIndice);
}

/*
 * [8] select_where
 * Executa 'n' buscas. Cada busca le 'm' e os m pares de criterios; decide
 * entre usar o indice (se codEstacao foi informado) ou a busca sequencial.
 */
void select_where(char *arquivoDados, char *arquivoIndice, int n)
{
    FILE *dados;

    /* O arquivo de dados e sempre necessario. */
    if (arquivoDados == NULL || !(dados = fopen(arquivoDados, "rb")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabDados;
    if (!header_reader(&cabDados, dados))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(dados);
        return;
    }

    /* Abre o indice e le seu cabecalho (necessario para buscas por chave). */
    FILE *indice = fopen(arquivoIndice, "rb");
    CabecalhoArvoreB cabArvore;
    int indiceOk = (indice != NULL) && cabecalhoArvore_ler(&cabArvore, indice);

    /* Processa cada uma das n buscas. */
    for (int i = 0; i < n; i++)
    {
        int m;
        scanf(" %d", &m);

        char vals[8][50];
        ler_criterios(m, vals);

        /* codEstacao presente => usa o indice arvore-B. */
        if (vals[0][0] != '\0')
        {
            if (!indiceOk)
                printf("Falha no processamento do arquivo.\n");
            else
                busca_por_indice(dados, &cabArvore, indice, vals);
        }
        else
        {
            /* Demais campos => percorre o arquivo de dados (func. [3]). */
            busca_sequencial(dados, cabDados.proxRRN, vals);
        }

        /* Linha em branco separando o resultado de cada busca. */
        printf("\n");
    }

    if (indice) fclose(indice);
    fclose(dados);
}
