#include "funcionalidades.h"
#include "cabecalho.h"
#include "registro.h"
#include "utilitarios.h"

/*
 * novaEstacao (auxiliar de [1]): verifica, no vetor de registros ja carregados
 * em memoria, se 'nome' ainda nao foi cadastrado. Retorna 1 se a estacao e
 * nova (deve contar em nroEstacoes), 0 se ja existe.
 */
static int novaEstacao(struct _dados **vetorDados, int n, const char *nome)
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(vetorDados[i]->nomeEstacao, nome) == 0)
            return 0;
    }
    return 1;
}

/*
 * [1] lerCsv
 * Le um arquivo CSV e gera o arquivo binario correspondente.
 * Estrategia de consistencia: o cabecalho so e gravado em disco ao final da
 * execucao, ja com status = '1'. Assim, um arquivo incompleto nunca aparece
 * como consistente.
 */
void lerCsv()
{
    char nomeCsv[100], nomeBin[100], *info, skip[102];
    scanf("%s %s", nomeCsv, nomeBin);

    FILE *csv = fopen(nomeCsv, "r");
    if (csv == NULL)
    {
        printf("Falha no processamento do arquivo.");
        return;
    }

    FILE *bin = fopen(nomeBin, "wb");
    if (bin == NULL)
    {
        printf("Falha no processamento do arquivo.");
        fclose(csv);
        return;
    }

    Cabecalho *cabecalho = criarCabecalho(); /* nasce com status '0' */
    struct _dados **vetorDados = NULL;

    fgets(skip, 102, csv); /* descarta a linha de cabecalho do CSV */

    /* Loop principal de leitura: um registro por linha do CSV. */
    while (1)
    {
        /* codEstacao (chave primaria) - tambem detecta o fim do arquivo. */
        info = lerInfo(csv);
        if (info == NULL)
            break;
        struct _dados *novoDado = criarDados();

        novoDado->codEstacao = atoi(info);
        free(info);

        /* nomeEstacao (obrigatorio). */
        info = lerInfo(csv);
        novoDado->tamNomeEstacao = strlen(info);
        novoDado->nomeEstacao = (char *)malloc((strlen(info) + 1) * sizeof(char));
        strcpy(novoDado->nomeEstacao, info);
        /* Conta a estacao apenas se o nome ainda nao apareceu. */
        if (novaEstacao(vetorDados, cabecalho->proxRRN, info))
            cabecalho->nroEstacoes++;
        free(info);

        /* codLinha (opcional). */
        info = lerInfo(csv);
        if (strlen(info) > 0)
            novoDado->codLinha = atoi(info);
        free(info);

        /* nomeLinha (opcional). */
        info = lerInfo(csv);
        if (strlen(info) > 0)
        {
            novoDado->tamNomelinha = strlen(info);
            novoDado->nomeLinha = (char *)malloc((strlen(info) + 1) * sizeof(char));
            strcpy(novoDado->nomeLinha, info);
        }
        free(info);

        /* codProxEstacao (opcional) - quando presente, forma um par. */
        info = lerInfo(csv);
        if (strlen(info) > 0)
        {
            novoDado->codProxEstacao = atoi(info);
            cabecalho->nroPares++;
        }
        free(info);

        /* distProxEstacao (opcional). */
        info = lerInfo(csv);
        if (strlen(info) > 0)
            novoDado->distProxEstacao = atoi(info);
        free(info);

        /* codLinhaIntegra (opcional). */
        info = lerInfo(csv);
        if (strlen(info) > 0)
            novoDado->codLinhaIntegra = atoi(info);
        free(info);

        /* codEstIntegra (opcional). */
        info = lerInfo(csv);
        if (info && strlen(info) > 0)
            novoDado->codEstIntegra = atoi(info);
        free(info);

        /* Le e descarta a quebra de linha final. */
        info = lerInfo(csv);
        free(info);

        /* Insere o novo registro no vetor em memoria. */
        cabecalho->proxRRN++;
        vetorDados = (struct _dados **)realloc(vetorDados, cabecalho->proxRRN * sizeof(struct _dados *));
        vetorDados[cabecalho->proxRRN - 1] = novoDado;

        if (info == NULL)
            break;
    }

    /* Grava primeiro a area de registros, com tamanho fixo de 80 bytes cada. */
    fseek(bin, TAM_CABECALHO, SEEK_SET);
    for (int i = 0; i < cabecalho->proxRRN; i++)
    {
        /* Campos de tamanho fixo. */
        fwrite(&vetorDados[i]->removido, sizeof(char), 1, bin);
        fwrite(&vetorDados[i]->proximo, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codLinha, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codProxEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->distProxEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codLinhaIntegra, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codEstIntegra, sizeof(int), 1, bin);

        /* Campos de tamanho variavel, precedidos por seus tamanhos. */
        fwrite(&vetorDados[i]->tamNomeEstacao, sizeof(int), 1, bin);
        fwrite(vetorDados[i]->nomeEstacao, sizeof(char), vetorDados[i]->tamNomeEstacao, bin);
        fwrite(&vetorDados[i]->tamNomelinha, sizeof(int), 1, bin);
        fwrite(vetorDados[i]->nomeLinha, sizeof(char), vetorDados[i]->tamNomelinha, bin);

        /* Completa o registro ate 80 bytes com lixo '$'. */
        int usados = TAM_FIXO_REGISTRO + vetorDados[i]->tamNomeEstacao + vetorDados[i]->tamNomelinha;
        for (int j = 0; j < TAM_REGISTRO - usados; j++)
            fputc('$', bin);
    }

    /* So agora, ao final, o cabecalho e gravado em disco com status '1'. */
    cabecalho->status = '1';
    fseek(bin, 0, SEEK_SET);
    escreverCabecalho(cabecalho, bin);

    fclose(csv);
    fclose(bin);

    BinarioNaTela(nomeBin);

    liberarVetorDados(vetorDados, cabecalho->proxRRN);
    free(cabecalho);
}

/*
 * [2] select_from
 * Recupera e exibe todos os registros validos do arquivo binario.
 * Regras: ignora registros removidos e traduz campos ausentes para "NULO".
 */
void select_from(char *arquivoEntrada)
{
    FILE *input_file;

    /* Validacao do arquivo de entrada. */
    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    /* Leitura e validacao do cabecalho (arquivo precisa estar consistente). */
    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    int found = 0; /* indica se ao menos um registro valido foi exibido */

    /* Loop principal: le registro a registro ate o fim do arquivo. */
    while (1)
    {
        Dados data;

        if (!data_reader(&data, input_file))
            break;

        /* Pula registros logicamente removidos. */
        if (data.removido == '1')
        {
            if (data.nomeEstacao)
                free(data.nomeEstacao);
            if (data.nomeLinha)
                free(data.nomeLinha);
            continue;
        }

        found = 1;
        printDados(&data);
        printf("\n");

        /* Libera as strings alocadas pela leitura. */
        if (data.nomeEstacao)
            free(data.nomeEstacao);
        if (data.nomeLinha)
            free(data.nomeLinha);
    }

    if (!found)
        printf("Registro inexistente.\n");

    fclose(input_file);
}

/*
 * [3] busca
 * Executa qntBuscas buscas independentes sobre o arquivo binario. Em cada
 * busca, le os criterios e percorre os registros validos.
 * Otimizacoes exigidas:
 *   - registros removidos sao pulados assim que o status removido e lido;
 *   - como codEstacao e chave primaria, ao encontra-lo a varredura para.
 */
void busca(char *arquivoEntrada, int qntBuscas)
{
    FILE *input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb")))
    {
        printf("Falha no processamento do arquivo.");
        return;
    }

    Cabecalho cabecalho;

    fread(&cabecalho.status, sizeof(char), 1, input_file);
    if (cabecalho.status == '0')
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    fread(&cabecalho.topo, sizeof(int), 1, input_file);
    fread(&cabecalho.proxRRN, sizeof(int), 1, input_file);
    fread(&cabecalho.nroEstacoes, sizeof(int), 1, input_file);
    fread(&cabecalho.nroPares, sizeof(int), 1, input_file);

    for (int i = 0; i < qntBuscas; i++)
    {
        int qntCampos;
        scanf(" %d", &qntCampos);

        char campo[50], valor[50];
        char vals[8][50] = {{0}};

        /* Leitura dos criterios desta busca. */
        for (int j = 0; j < qntCampos; j++)
        {
            scanf(" %s", campo);
            if (!strcmp(campo, "codEstacao"))
            {
                scanf(" %s", valor);
                strcpy(vals[0], valor);
            }
            else if (!strcmp(campo, "nomeEstacao"))
            {
                ScanQuoteString(valor);
                strcpy(vals[1], valor);
            }
            else if (!strcmp(campo, "codLinha"))
            {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[2], valor);
            }
            else if (!strcmp(campo, "nomeLinha"))
            {
                ScanQuoteString(valor);
                strcpy(vals[3], valor);
            }
            else if (!strcmp(campo, "codProxEstacao"))
            {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[4], valor);
            }
            else if (!strcmp(campo, "distProxEstacao"))
            {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[5], valor);
            }
            else if (!strcmp(campo, "codLinhaIntegra"))
            {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[6], valor);
            }
            else if (!strcmp(campo, "codEstIntegra"))
            {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[7], valor);
            }
        }

        Dados *dados = (Dados *)malloc(sizeof(Dados));
        int existe = 0;

        /* Varredura sequencial dos registros. */
        for (int j = 0; j < cabecalho.proxRRN; j++)
        {
            /* Le apenas o byte de status; se removido, pula os 79 bytes restantes. */
            if (fread(&dados->removido, sizeof(char), 1, input_file) != 1)
                break;
            if (dados->removido == '1')
            {
                fseek(input_file, TAM_REGISTRO - 1, SEEK_CUR);
                continue;
            }

            /* Registro ativo: le os demais campos. */
            fread(&dados->proximo, sizeof(int), 1, input_file);
            fread(&dados->codEstacao, sizeof(int), 1, input_file);
            fread(&dados->codLinha, sizeof(int), 1, input_file);
            fread(&dados->codProxEstacao, sizeof(int), 1, input_file);
            fread(&dados->distProxEstacao, sizeof(int), 1, input_file);
            fread(&dados->codLinhaIntegra, sizeof(int), 1, input_file);
            fread(&dados->codEstIntegra, sizeof(int), 1, input_file);
            fread(&dados->tamNomeEstacao, sizeof(int), 1, input_file);

            if (dados->tamNomeEstacao > 0)
            {
                dados->nomeEstacao = malloc(dados->tamNomeEstacao + 1);
                fread(dados->nomeEstacao, sizeof(char), dados->tamNomeEstacao, input_file);
                dados->nomeEstacao[dados->tamNomeEstacao] = '\0';
            }
            else
            {
                dados->nomeEstacao = NULL;
            }

            fread(&dados->tamNomelinha, sizeof(int), 1, input_file);

            if (dados->tamNomelinha > 0)
            {
                dados->nomeLinha = malloc(dados->tamNomelinha + 1);
                fread(dados->nomeLinha, sizeof(char), dados->tamNomelinha, input_file);
                dados->nomeLinha[dados->tamNomelinha] = '\0';
            }
            else
            {
                dados->nomeLinha = NULL;
            }

            /* Pula os bytes de lixo ('$') ate completar os 80 bytes do registro. */
            fseek(input_file, TAM_REGISTRO - TAM_FIXO_REGISTRO - dados->tamNomeEstacao - dados->tamNomelinha, SEEK_CUR);

            int casou = match_registro(dados, vals);
            if (casou)
            {
                printDados(dados);
                printf("\n");
                existe = 1;
            }

            if (dados->nomeEstacao)
                free(dados->nomeEstacao);
            if (dados->nomeLinha)
                free(dados->nomeLinha);

            /* codEstacao e chave primaria: encontrado o registro, para de percorrer. */
            if (casou && vals[0][0] != 0)
                break;
        }

        if (!existe)
            printf("Registro inexistente.\n");
        free(dados);
        printf("\n");

        /* Reposiciona no inicio dos registros para a proxima busca. */
        if (i < qntBuscas - 1)
            fseek(input_file, TAM_CABECALHO, SEEK_SET);
    }

    fclose(input_file);
}

/*
 * [4] delete_from
 * Remove logicamente os registros que casam com os criterios informados,
 * encadeando-os na pilha de removidos (campo topo/proximo) e atualizando os
 * contadores do cabecalho.
 */
void delete_from(char *arquivoEntrada)
{
    FILE *input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb+")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    /* Marca o arquivo como inconsistente enquanto a remocao acontece. */
    char status = '0';
    fseek(input_file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, input_file);

    int n;
    if (scanf("%d", &n) != 1)
    {
        fclose(input_file);
        return;
    }

    for (int j = 0; j < n; j++)
    {
        int m;
        scanf("%d", &m);

        char vals[8][50] = {{0}};
        char campo[50], valor[100];

        /* Leitura dos criterios de remocao desta operacao. */
        for (int k = 0; k < m; k++)
        {
            scanf("%s", campo);

            if (!strcmp(campo, "nomeEstacao") || !strcmp(campo, "nomeLinha"))
                ScanQuoteString(valor);
            else
            {
                scanf("%s", valor);
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

        /* Posiciona no primeiro registro (logo apos o cabecalho). */
        fseek(input_file, TAM_CABECALHO, SEEK_SET);

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long pos = ftell(input_file);

            Dados data;
            if (!data_reader(&data, input_file))
                break;

            /* Pula registros ja removidos. */
            if (data.removido == '1')
            {
                if (data.nomeEstacao) free(data.nomeEstacao);
                if (data.nomeLinha) free(data.nomeLinha);
                continue;
            }

            /* Encontrou um registro que casa: faz a remocao logica. */
            if (match_registro(&data, vals))
            {
                fseek(input_file, pos, SEEK_SET);

                /* Marca como removido e encadeia na pilha de removidos. */
                char removido = '1';
                fwrite(&removido, sizeof(char), 1, input_file);
                fwrite(&cabecalho.topo, sizeof(int), 1, input_file); /* proximo = topo antigo */
                cabecalho.topo = rrn;                                /* novo topo */

                /* Atualiza contadores do cabecalho. */
                if (data.codProxEstacao != -1)
                    cabecalho.nroPares--;
                if (!tem_estacao_ativa(input_file, cabecalho.proxRRN, data.nomeEstacao))
                    cabecalho.nroEstacoes--;

                /* Volta para a posicao de leitura do proximo registro. */
                fseek(input_file, pos + TAM_REGISTRO, SEEK_SET);
            }

            if (data.nomeEstacao) free(data.nomeEstacao);
            if (data.nomeLinha) free(data.nomeLinha);
        }
    }

    /* Regrava o cabecalho atualizado, ja consistente. */
    cabecalho.status = '1';
    fseek(input_file, 0, SEEK_SET);
    escreverCabecalho(&cabecalho, input_file);

    fclose(input_file);

    BinarioNaTela(arquivoEntrada);
}

/*
 * [5] inserir
 * Insere qntInsercoes novos registros no arquivo binario. Cada registro e
 * lido da entrada padrao na ordem fixa dos campos (NULO para campos ausentes).
 * Politica de alocacao: reaproveita o registro do topo da pilha de removidos
 * (cabecalho.topo) quando ha algum; caso contrario, anexa ao final do arquivo.
 */
void inserir(char *arquivoEntrada, int qntInsercoes)
{
    FILE *input_file;

    /* Abre em leitura+escrita binaria: a insercao altera o proprio arquivo. */
    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb+")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    /* Le e valida o cabecalho; o arquivo precisa estar consistente. */
    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    /* Marca o arquivo como inconsistente enquanto as insercoes ocorrem. */
    char status = '0';
    fseek(input_file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, input_file);

    for (int i = 0; i < qntInsercoes; i++)
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
            cabecalho.nroPares++;
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
        if (!tem_estacao_ativa(input_file, cabecalho.proxRRN, novoDado.nomeEstacao))
            cabecalho.nroEstacoes++;

        /* Decide onde gravar o registro. */
        if (cabecalho.topo == -1)
        {
            /* Pilha de removidos vazia: anexa ao final, gerando um novo RRN. */
            fseek(input_file, 0, SEEK_END);
            cabecalho.proxRRN++;
        }
        else
        {
            /* Reaproveita o slot do topo da pilha de removidos.
             * Posiciona no campo 'proximo' (apos o byte de removido) para
             * desempilhar: o novo topo passa a ser o 'proximo' do slot reusado. */
            int rrnReuso = cabecalho.topo;
            fseek(input_file, TAM_CABECALHO + (long)TAM_REGISTRO * rrnReuso + 1, SEEK_SET);
            int proximo;
            fread(&proximo, sizeof(int), 1, input_file);
            cabecalho.topo = proximo;
            if (rrnReuso == proximo) {
                cabecalho.nroEstacoes--;
                cabecalho.nroPares--;
            }
            /* Volta ao inicio do slot: foram lidos removido(1)+proximo(4)=5 bytes. */
            fseek(input_file, -5, SEEK_CUR);
        }

        /* Gravacao direta em disco do registro de tamanho fixo (80 bytes). */
        fwrite(&novoDado.removido, sizeof(char), 1, input_file);
        fwrite(&novoDado.proximo, sizeof(int), 1, input_file);
        fwrite(&novoDado.codEstacao, sizeof(int), 1, input_file);
        fwrite(&novoDado.codLinha, sizeof(int), 1, input_file);
        fwrite(&novoDado.codProxEstacao, sizeof(int), 1, input_file);
        fwrite(&novoDado.distProxEstacao, sizeof(int), 1, input_file);
        fwrite(&novoDado.codLinhaIntegra, sizeof(int), 1, input_file);
        fwrite(&novoDado.codEstIntegra, sizeof(int), 1, input_file);
        fwrite(&novoDado.tamNomeEstacao, sizeof(int), 1, input_file);
        fwrite(novoDado.nomeEstacao, sizeof(char), novoDado.tamNomeEstacao, input_file);
        fwrite(&novoDado.tamNomelinha, sizeof(int), 1, input_file);
        fwrite(novoDado.nomeLinha, sizeof(char), novoDado.tamNomelinha, input_file);

        /* Completa o registro ate 80 bytes com lixo '$'. */
        int usados = TAM_FIXO_REGISTRO + novoDado.tamNomeEstacao + novoDado.tamNomelinha;
        for (int j = 0; j < TAM_REGISTRO - usados; j++)
            fputc('$', input_file);

        /* Libera as strings alocadas para este registro. */
        free(novoDado.nomeEstacao);
        free(novoDado.nomeLinha);
    }

    /* So ao final o cabecalho e regravado, ja consistente. E obrigatorio
     * reposicionar em 0 antes de gravar (o ponteiro esta no fim de um registro). */
    cabecalho.status = '1';
    fseek(input_file, 0, SEEK_SET);
    escreverCabecalho(&cabecalho, input_file);

    fclose(input_file);
    BinarioNaTela(arquivoEntrada);
}

/*
 * [6] update
 * Atualiza campos dos registros que casam com os criterios. A operacao le
 * pares (campo, valor) de busca e pares de atualizacao; cada registro
 * atualizado e regravado apenas se ainda couber nos 80 bytes do registro.
 */
void update(char *arquivoEntrada)
{
    FILE *input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb+")))
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    /* Marca o arquivo como inconsistente durante a atualizacao. */
    char status = '0';
    fseek(input_file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, input_file);

    int n;
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        int m;
        scanf("%d", &m);

        char vals[8][50] = {{0}};
        char campo[50], valor[100];

        /* Le os criterios de selecao dos registros a atualizar. */
        for (int j = 0; j < m; j++)
        {
            scanf("%s", campo);

            if (!strcmp(campo, "nomeEstacao") || !strcmp(campo, "nomeLinha"))
                ScanQuoteString(valor);
            else
            {
                scanf("%s", valor);
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

        /* Le os pares (campo, novo valor) a aplicar. */
        int p;
        scanf("%d", &p);

        char campos[10][50];
        char valores[10][100];

        for (int k = 0; k < p; k++)
        {
            scanf("%s", campos[k]);

            if (!strcmp(campos[k], "nomeEstacao") || !strcmp(campos[k], "nomeLinha"))
                ScanQuoteString(valores[k]);
            else
                scanf("%s", valores[k]);
        }

        /* Posiciona no primeiro registro. */
        fseek(input_file, TAM_CABECALHO, SEEK_SET);

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long pos = ftell(input_file);

            Dados data;
            if (!data_reader(&data, input_file))
                break;

            /* Pula removidos e registros que nao casam com os criterios. */
            if (data.removido == '1' || !match_registro(&data, vals))
            {
                if (data.nomeEstacao) free(data.nomeEstacao);
                if (data.nomeLinha) free(data.nomeLinha);
                continue;
            }

            /* Copia as strings atuais para buffers locais editaveis. */
            char nomeEstacao[100] = {0};
            char nomeLinha[100] = {0};

            if (data.tamNomeEstacao > 0)
                strcpy(nomeEstacao, data.nomeEstacao);
            if (data.tamNomelinha > 0)
                strcpy(nomeLinha, data.nomeLinha);

            /* Aplica cada atualizacao solicitada. */
            for (int k = 0; k < p; k++)
            {
                if (!strcmp(campos[k], "codLinha"))
                    data.codLinha = atoi(valores[k]);
                else if (!strcmp(campos[k], "codProxEstacao"))
                    data.codProxEstacao = atoi(valores[k]);
                else if (!strcmp(campos[k], "distProxEstacao"))
                    data.distProxEstacao = atoi(valores[k]);
                else if (!strcmp(campos[k], "codLinhaIntegra"))
                    data.codLinhaIntegra = atoi(valores[k]);
                else if (!strcmp(campos[k], "codEstIntegra"))
                    data.codEstIntegra = atoi(valores[k]);
                else if (!strcmp(campos[k], "nomeEstacao"))
                {
                    if (strcmp(valores[k], "") == 0)
                    {
                        data.tamNomeEstacao = 0;
                        nomeEstacao[0] = '\0';
                    }
                    else
                    {
                        data.tamNomeEstacao = strlen(valores[k]);
                        strcpy(nomeEstacao, valores[k]);
                    }
                }
                else if (!strcmp(campos[k], "nomeLinha"))
                {
                    if (strcmp(valores[k], "") == 0)
                    {
                        data.tamNomelinha = 0;
                        nomeLinha[0] = '\0';
                    }
                    else
                    {
                        data.tamNomelinha = strlen(valores[k]);
                        strcpy(nomeLinha, valores[k]);
                    }
                }
            }

            int novo_tam = TAM_FIXO_REGISTRO + data.tamNomeEstacao + data.tamNomelinha;

            /* So regrava se o registro atualizado ainda couber nos 80 bytes. */
            if (novo_tam <= TAM_REGISTRO)
            {
                fseek(input_file, pos, SEEK_SET);

                fwrite(&data.removido, sizeof(char), 1, input_file);
                fwrite(&data.proximo, sizeof(int), 1, input_file);
                fwrite(&data.codEstacao, sizeof(int), 1, input_file);
                fwrite(&data.codLinha, sizeof(int), 1, input_file);
                fwrite(&data.codProxEstacao, sizeof(int), 1, input_file);
                fwrite(&data.distProxEstacao, sizeof(int), 1, input_file);
                fwrite(&data.codLinhaIntegra, sizeof(int), 1, input_file);
                fwrite(&data.codEstIntegra, sizeof(int), 1, input_file);

                fwrite(&data.tamNomeEstacao, sizeof(int), 1, input_file);
                fwrite(nomeEstacao, sizeof(char), data.tamNomeEstacao, input_file);
                fwrite(&data.tamNomelinha, sizeof(int), 1, input_file);
                fwrite(nomeLinha, sizeof(char), data.tamNomelinha, input_file);

                /* Completa novamente ate 80 bytes com lixo '$'. */
                int preenchido = novo_tam;
                while (preenchido < TAM_REGISTRO)
                {
                    fputc('$', input_file);
                    preenchido++;
                }
            }

            if (data.nomeEstacao) free(data.nomeEstacao);
            if (data.nomeLinha) free(data.nomeLinha);
        }
    }

    /* Regrava o cabecalho, agora consistente. */
    cabecalho.status = '1';
    fseek(input_file, 0, SEEK_SET);
    escreverCabecalho(&cabecalho, input_file);

    fclose(input_file);

    BinarioNaTela(arquivoEntrada);
}
