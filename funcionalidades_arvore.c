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

void insert_arvore(char *arquivoDados, char *arquivoIndice, int n) {
    FILE *dados;

    /* O arquivo de dados e sempre necessario. */
    if (arquivoDados == NULL || !(dados = fopen(arquivoDados, "rb+")))
    {
        printf("Falha no processamento do arquivo1.\n");
        return;
    }
    Cabecalho cabDados;
    if (!header_reader(&cabDados, dados))
    {
        printf("Falha no processamento do arquivo2.\n");
        fclose(dados);
        return;
    }

    FILE *indice;
    if (arquivoIndice == NULL || !(indice = fopen(arquivoIndice, "rb+")))
    {
        printf("Falha no processamento do arquivo3.\n");
        fclose(dados);
        return;
    }
    CabecalhoArvoreB cabArvore;
    if (!cabecalhoArvore_ler(&cabArvore, indice))
    {
        printf("Falha no processamento do arquivo4.\n");
        fclose(dados);
        fclose(indice);
        return;
    }

    /* Marca os arquivos como inconsistentes enquanto a insercao acontece. */
    char status = '0';
    fseek(dados, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, dados);
    fseek(indice, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, indice);

    for (int i = 0; i < n; i++)
    {
        /* Novo registro inicializado com valores neutros: ativo ('0') e sem
         * encadeamento (proximo = -1). */
        Dados novoDado;
        novoDado.removido = '0';
        novoDado.proximo = -1;
        novoDado.codEstacao = -1;
        novoDado.codLinha = -1;
        novoDado.codProxEstacao = -1;
        novoDado.distProxEstacao = -1;
        novoDado.codLinhaIntegra = -1;
        novoDado.codEstIntegra = -1;
        novoDado.tamNomeEstacao = 0;
        novoDado.nomeEstacao = NULL;
        novoDado.tamNomelinha = 0;
        novoDado.nomeLinha = NULL;

        char valor[100];

        /* codEstacao (chave primaria, obrigatoria). */
        scanf(" %s", valor);
        novoDado.codEstacao = atoi(valor);

        if (arvore_buscar(indice, &cabArvore, novoDado.codEstacao) != -1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF); /* limpa o buffer de entrada */
            continue;
        }

        /* nomeEstacao (obrigatorio; pode vir entre aspas). */
        ScanQuoteString(valor);
        novoDado.tamNomeEstacao = strlen(valor);
        novoDado.nomeEstacao = (char *)malloc((strlen(valor) + 1) * sizeof(char));
        strcpy(novoDado.nomeEstacao, valor);

        /* codLinha (opcional). */
        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codLinha = atoi(valor);

        /* nomeLinha (opcional). ScanQuoteString devolve "" para NULO, por isso
         * tambem testamos string vazia para manter nomeLinha nulo. */
        ScanQuoteString(valor);
        if (strcmp(valor, "NULO") != 0 && valor[0] != '\0')
        {
            novoDado.tamNomelinha = strlen(valor);
            novoDado.nomeLinha = (char *)malloc((strlen(valor) + 1) * sizeof(char));
            strcpy(novoDado.nomeLinha, valor);
        }

        /* codProxEstacao (opcional) - quando presente, forma um par. */
        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
        {
            novoDado.codProxEstacao = atoi(valor);
            cabDados.nroPares++;
        }

        /* distProxEstacao (opcional). */
        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.distProxEstacao = atoi(valor);

        /* codLinhaIntegra (opcional). */
        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codLinhaIntegra = atoi(valor);

        /* codEstIntegra (opcional). */
        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codEstIntegra = atoi(valor);

        /* Conta uma nova estacao apenas se o nome ainda nao estiver ativo no
         * arquivo (simetrico a remocao em delete_from). */
        if (!tem_estacao_ativa(dados, cabDados.proxRRN, novoDado.nomeEstacao))
            cabDados.nroEstacoes++;

        /* Decide onde gravar o registro. */
        if (cabDados.topo == -1)
        {
            /* Pilha de removidos vazia: anexa ao final, gerando um novo RRN. */
            fseek(dados, 0, SEEK_END);
            cabDados.proxRRN++;
            arvore_inserir(indice, &cabArvore, novoDado.codEstacao, (int)(TAM_CABECALHO + (long)TAM_REGISTRO * (cabDados.proxRRN - 1)));
        }
        else
        {
            /* Reaproveita o slot do topo da pilha de removidos.
             * Posiciona no campo 'proximo' (apos o byte de removido) para
             * desempilhar: o novo topo passa a ser o 'proximo' do slot reusado. */
            int rrnReuso = cabDados.topo;
            fseek(dados, TAM_CABECALHO + (long)TAM_REGISTRO * rrnReuso + 1, SEEK_SET);
            int proximo;
            fread(&proximo, sizeof(int), 1, dados);
            cabDados.topo = proximo;
            if (rrnReuso == proximo) {
                cabDados.nroEstacoes--;
                cabDados.nroPares--;
            }
            /* Volta ao inicio do slot: foram lidos removido(1)+proximo(4)=5 bytes. */
            fseek(dados, -5, SEEK_CUR);
            arvore_inserir(indice, &cabArvore, novoDado.codEstacao, (int)(TAM_CABECALHO + (long)TAM_REGISTRO * rrnReuso));
        }

        /* Gravacao direta em disco do registro de tamanho fixo (80 bytes). */
        fwrite(&novoDado.removido, sizeof(char), 1, dados);
        fwrite(&novoDado.proximo, sizeof(int), 1, dados);
        fwrite(&novoDado.codEstacao, sizeof(int), 1, dados);
        fwrite(&novoDado.codLinha, sizeof(int), 1, dados);
        fwrite(&novoDado.codProxEstacao, sizeof(int), 1, dados);
        fwrite(&novoDado.distProxEstacao, sizeof(int), 1, dados);
        fwrite(&novoDado.codLinhaIntegra, sizeof(int), 1, dados);
        fwrite(&novoDado.codEstIntegra, sizeof(int), 1, dados);
        fwrite(&novoDado.tamNomeEstacao, sizeof(int), 1, dados);
        fwrite(novoDado.nomeEstacao, sizeof(char), novoDado.tamNomeEstacao, dados);
        fwrite(&novoDado.tamNomelinha, sizeof(int), 1, dados);
        fwrite(novoDado.nomeLinha, sizeof(char), novoDado.tamNomelinha, dados);

        /* Completa o registro ate 80 bytes com lixo '$'. */
        int usados = TAM_FIXO_REGISTRO + novoDado.tamNomeEstacao + novoDado.tamNomelinha;
        for (int j = 0; j < TAM_REGISTRO - usados; j++)
            fputc('$', dados);

        /* Libera as strings alocadas para este registro. */
        free(novoDado.nomeEstacao);
        free(novoDado.nomeLinha);
    }

    cabDados.status = '1';
    fseek(dados, 0, SEEK_SET);
    escreverCabecalho(&cabDados, dados);
    cabArvore.status = '1';
    cabecalhoArvore_escrever(&cabArvore, indice);

    fclose(dados);
    fclose(indice);
    BinarioNaTela(arquivoDados);
    BinarioNaTela(arquivoIndice);
}

void delete_arvore(char *arquivoDados, char *arquivoIndice, int n) {
    FILE *dados;
}