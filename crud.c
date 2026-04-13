#include "crud.h"

void criar_dados(Dados* dados)
{
    dados->removido = '0';
    dados->proximo = -1;
    dados->codEstacao = -1;
    dados->codLinha = -1;
    dados->codProxEstacao = -1;
    dados->distProxEstacao = -1;
    dados->codLinhaIntegra = -1;
    dados->codEstIntegra = -1;
    dados->tamNomeEstacao = 0;
    dados->nomeEstacao = NULL;
    dados->tamNomelinha = 0;
    dados->nomeLinha = NULL;
}

void liberar_dados(Dados* dados)
{
    if (dados->nomeEstacao) 
    {
        free(dados->nomeEstacao);
        dados->nomeEstacao = NULL;
    }
    if (dados->nomeLinha) {
        free(dados->nomeLinha);
        dados->nomeLinha = NULL;
    }
}

FILE* abrir_arquivo(char* arquivo, char* modo)
{
    if (arquivo == NULL)
        return NULL;

    FILE* f = fopen(arquivo, modo);

    if (!f)
        printf("Falha no processamento do arquivo.\n");

    return f;
}

int ler_cabecalho(Cabecalho* cab, FILE* input_file)
{
    if (cab == NULL || input_file == NULL)
        return 0;

    if (fread(&cab->status, sizeof(char), 1, input_file) != 1 ||
        fread(&cab->topo, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->proxRRN, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->nroEstacoes, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->nroPares, sizeof(int), 1, input_file) != 1)
        return 0;

    return (cab->status == '1');
}

void escrever_cabecalho(Cabecalho* cab, FILE* f)
{
    fseek(f, 0, SEEK_SET);
    fwrite(&cab->status, sizeof(char), 1, f);
    fwrite(&cab->topo, sizeof(int), 1, f);
    fwrite(&cab->proxRRN, sizeof(int), 1, f);
    fwrite(&cab->nroEstacoes, sizeof(int), 1, f);
    fwrite(&cab->nroPares, sizeof(int), 1, f);
}

int ler_dados(Dados* data, FILE* input_file)
{
    if (fread(&data->removido, sizeof(char), 1, input_file) != 1)
        return 0;

    fread(&data->proximo, sizeof(int), 1, input_file);
    fread(&data->codEstacao, sizeof(int), 1, input_file);
    fread(&data->codLinha, sizeof(int), 1, input_file);
    fread(&data->codProxEstacao, sizeof(int), 1, input_file);
    fread(&data->distProxEstacao, sizeof(int), 1, input_file);
    fread(&data->codLinhaIntegra, sizeof(int), 1, input_file);
    fread(&data->codEstIntegra, sizeof(int), 1, input_file);
    fread(&data->tamNomeEstacao, sizeof(int), 1, input_file);

    if (data->removido == '1') 
    {
        fseek(input_file, 80 - 33, SEEK_CUR);
        data->nomeEstacao = NULL;
        data->nomeLinha = NULL;
        data->tamNomeEstacao = 0;
        data->tamNomelinha = 0;
        return 1;
    }

    if (data->tamNomeEstacao > 0) {
        data->nomeEstacao = malloc(data->tamNomeEstacao + 1);
        fread(data->nomeEstacao, sizeof(char), data->tamNomeEstacao, input_file);
        data->nomeEstacao[data->tamNomeEstacao] = '\0';
    }
    else {
        data->nomeEstacao = NULL;
    }

    fread(&data->tamNomelinha, sizeof(int), 1, input_file);

    if (data->tamNomelinha > 0) {
        data->nomeLinha = malloc(data->tamNomelinha + 1);
        fread(data->nomeLinha, sizeof(char), data->tamNomelinha, input_file);
        data->nomeLinha[data->tamNomelinha] = '\0';
    }
    else {
        data->nomeLinha = NULL;
    }

    // pula lixo: 80 - 37 - tamNomeEstacao - tamNomelinha
    int lixo_size = 80 - 37 - data->tamNomeEstacao - data->tamNomelinha;
    if (lixo_size > 0)
        fseek(input_file, lixo_size, SEEK_CUR);

    return 1;
}

void escrever_dados(Dados* data, FILE* f)
{
    fwrite(&data->removido, sizeof(char), 1, f);
    fwrite(&data->proximo, sizeof(int), 1, f);
    fwrite(&data->codEstacao, sizeof(int), 1, f);
    fwrite(&data->codLinha, sizeof(int), 1, f);
    fwrite(&data->codProxEstacao, sizeof(int), 1, f);
    fwrite(&data->distProxEstacao, sizeof(int), 1, f);
    fwrite(&data->codLinhaIntegra, sizeof(int), 1, f);
    fwrite(&data->codEstIntegra, sizeof(int), 1, f);
    fwrite(&data->tamNomeEstacao, sizeof(int), 1, f);

    if (data->tamNomeEstacao > 0 && data->nomeEstacao)
        fwrite(data->nomeEstacao, sizeof(char), data->tamNomeEstacao, f);

    fwrite(&data->tamNomelinha, sizeof(int), 1, f);

    if (data->tamNomelinha > 0 && data->nomeLinha)
        fwrite(data->nomeLinha, sizeof(char), data->tamNomelinha, f);

    int padding = 80 - 37 - data->tamNomeEstacao - data->tamNomelinha;
    for (int j = 0; j < padding; j++)
        fputc('$', f);
}

void set_status(FILE* f, char status)
{
    fseek(f, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, f);
}

int ler_info(FILE* csv, char buffer[100])
{
    int i = 0;
    int c;

    while (1) {
        c = fgetc(csv);

        if (c == ',' || c == '\n' || c == EOF || c == '\r')
            break;

        buffer[i++] = c;
    }

    buffer[i] = '\0';

    if (c == EOF)
        return -1;

    return i;
}

int match_registro(Dados* dados, char vals[8][50])
{
    // Verifica codEstacao
    // Se há filtro (vals[0][0] != '\0') e não bate → falha
    if (vals[0][0] && atoi(vals[0]) != dados->codEstacao)
        return 0;

    // Verifica nomeEstacao
    // Falha se filtro existe e:
    // - nome no registro é NULL
    // - ou diferente do filtro
    if (vals[1][0] && (!dados->nomeEstacao || strcmp(vals[1], dados->nomeEstacao) != 0))
        return 0;

    // Verifica codLinha
    if (vals[2][0] && atoi(vals[2]) != dados->codLinha)
        return 0;

    // Verifica nomeLinha
    if (vals[3][0] && (!dados->nomeLinha || strcmp(vals[3], dados->nomeLinha) != 0))
        return 0;

    // Verifica codProxEstacao
    if (vals[4][0] && atoi(vals[4]) != dados->codProxEstacao)
        return 0;

    // Verifica distProxEstacao
    if (vals[5][0] && atoi(vals[5]) != dados->distProxEstacao)
        return 0;

    // Verifica codLinhaIntegra
    if (vals[6][0] && atoi(vals[6]) != dados->codLinhaIntegra)
        return 0;

    // Verifica codEstIntegra
    if (vals[7][0] && atoi(vals[7]) != dados->codEstIntegra)
        return 0;

    // Garante que pelo menos UM filtro foi usado
    for (int i = 0; i < 8; i++)
        if (vals[i][0]) return 1;

    return 0;
}

void ler() //FUNCIONALIDADE 1
{
    char nomeCsv[100], nomeBin[100], buffer[100];
    scanf("%s %s", nomeCsv, nomeBin);

    FILE* csv;
    FILE* bin;

    if (!(csv = abrir_arquivo(nomeCsv, "r"))) return;
    if (!(bin = abrir_arquivo(nomeBin, "wb"))) {
        fclose(csv);
        return;
    }

    Cabecalho cabecalho;
    cabecalho.status = '1';
    cabecalho.topo = -1;
    cabecalho.proxRRN = 0;
    cabecalho.nroEstacoes = 0;
    cabecalho.nroPares = 0;

    Dados novoDado;
    char** nomes = NULL; // variavel usada para armazenar os nomes das estações e facilitar a contagem do nroEstacoes

    // pular a primeira linha do CSV
    fseek(csv, 101, SEEK_CUR);

    // reservar espaco para o cabecalho
    fwrite("_________________", sizeof(char), 17, bin);

    while (1) {
        // inicializar os campos com os valores "nulos"
        criar_dados(&novoDado);

        // INÍCIO DA SESSÃO DE LEITURA DOS CAMPOS

        ler_info(csv, buffer);
        novoDado.codEstacao = atoi(buffer);

        novoDado.tamNomeEstacao = ler_info(csv, buffer);
        novoDado.nomeEstacao = (char*)malloc((novoDado.tamNomeEstacao + 1) * sizeof(char));
        strcpy(novoDado.nomeEstacao, buffer);
        nomes = (char**)realloc(nomes, (cabecalho.proxRRN + 1) * sizeof(char*));
        nomes[cabecalho.proxRRN] = (char*)malloc((novoDado.tamNomeEstacao + 1) * sizeof(char));
        strcpy(nomes[cabecalho.proxRRN], buffer);
        if (nova_estacao(nomes, buffer, cabecalho.proxRRN))
            cabecalho.nroEstacoes++;

        // a partir daqui vamos sempre checar se o campo é nulo

        if (ler_info(csv, buffer) > 0)
            novoDado.codLinha = atoi(buffer);

        novoDado.tamNomelinha = ler_info(csv, buffer);
        if (novoDado.tamNomelinha > 0) {
            novoDado.nomeLinha = (char*)malloc((novoDado.tamNomelinha + 1) * sizeof(char));
            strcpy(novoDado.nomeLinha, buffer);
        }

        if (ler_info(csv, buffer) > 0) {
            novoDado.codProxEstacao = atoi(buffer);
            cabecalho.nroPares++; // se codProxEstacao não for nulo incrementamos o nroPares
        }

        if (ler_info(csv, buffer) > 0)
            novoDado.distProxEstacao = atoi(buffer);

        if (ler_info(csv, buffer) > 0)
            novoDado.codLinhaIntegra = atoi(buffer);

        if (ler_info(csv, buffer) > 0)
            novoDado.codEstIntegra = atoi(buffer);

        cabecalho.proxRRN++;

        // FIM DA LEITURA DOS CAMPOS, INÍCIO DA SESSÃO DE ESCRITA DO REGISTRO

        escrever_dados(&novoDado, bin);
        liberar_dados(&novoDado);

        // ler a quebra de linha, se na verdade for EOF encerrar a leitura
        if (ler_info(csv, buffer) == -1) break;
    }

    escrever_cabecalho(&cabecalho, bin);

    fclose(csv);
    fclose(bin);

    BinarioNaTela(nomeBin);

    for (int i = 0; i < cabecalho.proxRRN; i++)
        free(nomes[i]);
    free(nomes);
}

void selecionar(char* arquivoEntrada) //FUNCIONALIDADE 2
{
    // Abre arquivo binário para leitura
    FILE* input_file = abrir_arquivo(arquivoEntrada, "rb");

    if (!input_file)
        return;

    Cabecalho cabecalho;

    // Lê cabeçalho e valida consistência (status == '1')
    if (!ler_cabecalho(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    int found = 0;

    // Percorre todos os registros existentes
    for (int i = 0; i < cabecalho.proxRRN; i++)
    {
        Dados data;

        criar_dados(&data);

        // Lê registro
        if (!ler_dados(&data, input_file))
            break;

        // Ignora registros removidos
        if (data.removido == '1')
        {
            liberar_dados(&data);
            continue;
        }

        // Encontrou pelo menos um válido
        found = 1;

        // Imprime registro
        print_dados(&data);
        printf("\n");

        liberar_dados(&data);
    }

    // Caso não exista nenhum registro válido
    if (!found)
        printf("Registro inexistente.\n");

    fclose(input_file);
}

void buscar(char* arquivoEntrada, int qntBuscas) //FUNCIONALIDADE 3
{
    FILE* input_file = abrir_arquivo(arquivoEntrada, "rb");
    if (!input_file) return;

    Cabecalho cabecalho;
    if (!ler_cabecalho(&cabecalho, input_file)) {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    for (int i = 0; i < qntBuscas; i++) {
        int qntCampos;
        scanf(" %d", &qntCampos);

        char campo[50], valor[50];
        char vals[8][50] = { {0} };

        // verifica a entrada dos filtros e armazena no vetor "vals" os pares (campo,valor)
        for (int j = 0; j < qntCampos; j++)
            input_filtro(campo, valor, vals);

        int existe = 0; // para verificar se algum registro se encaixa nos filtros

        // busca no arquivo inteiro
        fseek(input_file, 17, SEEK_SET);
        for (int j = 0; j < cabecalho.proxRRN; j++) {
            Dados dados;
            criar_dados(&dados);

            if (!ler_dados(&dados, input_file)) break;

            if (dados.removido == '1') {
                liberar_dados(&dados);
                continue;
            }

            if (match_registro(&dados, vals)) {
                print_dados(&dados);
                printf("\n");
                existe = 1;
            }

            liberar_dados(&dados);
        }

        if (!existe)
            printf("Registro inexistente.\n");

        printf("\n");
    }

    fclose(input_file);
}

void deletar(char* arquivoEntrada) //FUNCIONALIDADE 4
{
    // Abre arquivo para leitura e escrita
    FILE* input_file = abrir_arquivo(arquivoEntrada, "rb+");

    if (!input_file)
        return;

    Cabecalho cabecalho;

    // Lê cabeçalho e valida consistência
    if (!ler_cabecalho(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    // Marca arquivo como inconsistente durante operação
    set_status(input_file, '0');

    int n;

    // Número de remoções
    if (scanf("%d", &n) != 1)
    {
        fclose(input_file);
        return;
    }

    // Para cada remoção
    for (int j = 0; j < n; j++)
    {
        int m;

        // Número de campos no filtro
        scanf("%d", &m);

        char vals[8][50] = { {0} };
        char campo[50], valor[100];

        // Lê os filtros
        for (int k = 0; k < m; k++)
            input_filtro(campo, valor, vals);

        // Vai para o início dos registros (após cabeçalho)
        fseek(input_file, 17, SEEK_SET);

        // Percorre todos os registros
        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long pos = ftell(input_file); // Guarda posição do registro
            Dados data;

            if (!ler_dados(&data, input_file))
                break;

            // Ignora removidos
            if (data.removido == '1')
            {
                liberar_dados(&data);
                continue;
            }

            // Se o registro atende ao filtro
            if (match_registro(&data, vals))
            {
                // Volta ao início do registro
                fseek(input_file, -80, SEEK_CUR);

                // Marca como removido
                char removido = '1';
                fwrite(&removido, sizeof(char), 1, input_file);

                // Encadeamento de lista de removidos (lista invertida)
                fwrite(&cabecalho.topo, sizeof(int), 1, input_file);

                // Atualiza topo da lista de removidos
                cabecalho.topo = rrn;

                // Atualiza estatísticas
                if (data.codProxEstacao != -1)
                    cabecalho.nroPares--;

                // Verifica se ainda existe estação ativa com esse nome
                if (!tem_estacao_ativa(input_file, cabecalho.proxRRN, data.nomeEstacao))
                    cabecalho.nroEstacoes--;

                // Avança para próximo registro
                fseek(input_file, 75, SEEK_CUR);
            }

            liberar_dados(&data);
        }
    }

    // Marca arquivo como consistente novamente
    cabecalho.status = '1';
    escrever_cabecalho(&cabecalho, input_file);

    fclose(input_file);

    // Exibe checksum do arquivo
    BinarioNaTela(arquivoEntrada);
}

void inserir(char* arquivoEntrada, int qntInsercoes)//FUNCIONALIDADE 5 
{
    FILE* input_file = abrir_arquivo(arquivoEntrada, "r+b");
    if (!input_file) return;

    // lê o cabecalho e encerra o programa se inconsistente, se não, define o status como inconsistente
    Cabecalho cabecalho;
    if (!ler_cabecalho(&cabecalho, input_file)) {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }
    set_status(input_file, '0');

    for (int i = 0; i < qntInsercoes; i++) {
        Dados novoDado;
        criar_dados(&novoDado);

        char valor[50];

        // lê cada input da entrada e já salva na struct auxiliar
        scanf(" %s", valor);
        novoDado.codEstacao = atoi(valor);

        ScanQuoteString(valor);
        novoDado.tamNomeEstacao = strlen(valor);
        novoDado.nomeEstacao = (char*)malloc((strlen(valor) + 1) * sizeof(char));
        strcpy(novoDado.nomeEstacao, valor);

        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codLinha = atoi(valor);

        ScanQuoteString(valor);
        if (strcmp(valor, "NULO") != 0) {
            novoDado.tamNomelinha = strlen(valor);
            novoDado.nomeLinha = (char*)malloc((strlen(valor) + 1) * sizeof(char));
            strcpy(novoDado.nomeLinha, valor);
        }

        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0) {
            novoDado.codProxEstacao = atoi(valor);
            cabecalho.nroPares++;
        }

        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.distProxEstacao = atoi(valor);

        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codLinhaIntegra = atoi(valor);

        scanf(" %s", valor);
        if (strcmp(valor, "NULO") != 0)
            novoDado.codEstIntegra = atoi(valor);

        if (cabecalho.topo == -1) {
            fseek(input_file, 0, SEEK_END);
            cabecalho.proxRRN++;
        }
        else {
            fseek(input_file, 17 + 80 * cabecalho.topo + 1, SEEK_SET);
            int proximo;
            fread(&proximo, sizeof(int), 1, input_file);
            cabecalho.topo = proximo;
            fseek(input_file, -5, SEEK_CUR);
        }

        // escreve tudo no arquivo
        escrever_dados(&novoDado, input_file);
        liberar_dados(&novoDado);
    }

    cabecalho.status = '1';
    escrever_cabecalho(&cabecalho, input_file);

    fclose(input_file);
    BinarioNaTela(arquivoEntrada);
}

void atualizar(char* arquivoEntrada)
{
    FILE* f = abrir_arquivo(arquivoEntrada, "r+b");

    if (!f)
        return;

    Cabecalho cab;

    if (!ler_cabecalho(&cab, f))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(f);
        return;
    }

    set_status(f, '0');

    int qntAtualizacoes;

    scanf(" %d", &qntAtualizacoes);

    for (int i = 0; i < qntAtualizacoes; i++)
    {
        // criterios busca
        int qntBusca;
        scanf(" %d", &qntBusca);

        char campo[50], valor[50];
        char vals_busca[8][50] = { {0} };

        for (int j = 0; j < qntBusca; j++)
            input_filtro(campo, valor, vals_busca);

        // campos atualização
        int qntAtualiza;
        scanf(" %d", &qntAtualiza);

        char vals_upd[8][50] = { {0} };
        int upd_ativo[8] = { 0 };

        for (int j = 0; j < qntAtualiza; j++)
        {
            int index = input_filtro(campo, valor, vals_upd);
            upd_ativo[index] = 1;
        }

        fseek(f, 17, SEEK_SET);

        for (int j = 0; j < cab.proxRRN; j++)
        {
            Dados dados;

            criar_dados(&dados);

            if (!ler_dados(&dados, f))
                break;

            if (dados.removido == '1')
            {
                liberar_dados(&dados);
                continue;
            }

            if (!match_registro(&dados, vals_busca))
            {
                liberar_dados(&dados);
                continue;
            }

            // aplica atualizações
            if (upd_ativo[0]) dados.codEstacao = atoi(vals_upd[0]);

            if (upd_ativo[1])
            {
                free(dados.nomeEstacao);
                dados.tamNomeEstacao = strlen(vals_upd[1]);
                dados.nomeEstacao = malloc(dados.tamNomeEstacao + 1);
                strcpy(dados.nomeEstacao, vals_upd[1]);
            }
            if (upd_ativo[2]) dados.codLinha = atoi(vals_upd[2]);

            if (upd_ativo[3])
            {
                free(dados.nomeLinha);
                dados.tamNomelinha = strlen(vals_upd[3]);
                dados.nomeLinha = malloc(dados.tamNomelinha + 1);
                strcpy(dados.nomeLinha, vals_upd[3]);
            }
            if (upd_ativo[4]) dados.codProxEstacao = atoi(vals_upd[4]);
            if (upd_ativo[5]) dados.distProxEstacao = atoi(vals_upd[5]);
            if (upd_ativo[6]) dados.codLinhaIntegra = atoi(vals_upd[6]);
            if (upd_ativo[7]) dados.codEstIntegra = atoi(vals_upd[7]);

            // reescreve in-place (volta 80 bytes)
            fseek(f, -80, SEEK_CUR);
            escrever_dados(&dados, f);
            // ponteiro já em 17+(j+1)*80, sem seek extra
            liberar_dados(&dados);
        }
    }

    cab.status = '1';
    escrever_cabecalho(&cab, f);
    fclose(f);
    BinarioNaTela(arquivoEntrada);
}

int nova_estacao(char** nomes, char* nome, int n)
{
    for (int i = 0; i < n; i++) {
        if (strcmp(nomes[i], nome) == 0)
            return 0;
    }
    return 1;
}

void print_dados(Dados* data)
{
    printf("%d ", data->codEstacao);
    printf("%s ", data->nomeEstacao);
    (data->codLinha == -1) ? printf("NULO ") : printf("%d ", data->codLinha);
    (data->tamNomelinha == 0) ? printf("NULO ") : printf("%s ", data->nomeLinha);
    (data->codProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->codProxEstacao);
    (data->distProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->distProxEstacao);
    (data->codLinhaIntegra == -1) ? printf("NULO ") : printf("%d ", data->codLinhaIntegra);
    (data->codEstIntegra == -1) ? printf("NULO") : printf("%d", data->codEstIntegra);
}

// 
int input_filtro(char campo[50], char valor[50], char vals[8][50])
{
    int index = -1;
    scanf("%s", campo);

    if (!strcmp(campo, "nomeEstacao") || !strcmp(campo, "nomeLinha"))
        ScanQuoteString(valor);
    else {
        scanf("%s", valor);
        if (strcmp(valor, "NULO") == 0)
            strcpy(valor, "-1");
    }

    if (!strcmp(campo, "codEstacao"))           index = 0;
    else if (!strcmp(campo, "nomeEstacao"))     index = 1;
    else if (!strcmp(campo, "codLinha"))        index = 2;
    else if (!strcmp(campo, "nomeLinha"))       index = 3;
    else if (!strcmp(campo, "codProxEstacao"))  index = 4;
    else if (!strcmp(campo, "distProxEstacao")) index = 5;
    else if (!strcmp(campo, "codLinhaIntegra")) index = 6;
    else if (!strcmp(campo, "codEstIntegra"))   index = 7;

    strcpy(vals[index], valor);
    return index;
}

int tem_estacao_ativa(FILE* input_file, int proxRRN, char* nome)
{
    // Define o nome a ser buscado (evita NULL)
    char* nome_busca = nome ? nome : "";

    // Salva posição atual do ponteiro do arquivo
    long pos_atual = ftell(input_file);

    int existe = 0; // Flag de existência

    // Vai para o início dos registros (após cabeçalho de 17 bytes)
    fseek(input_file, 17, SEEK_SET);

    // Percorre todos os registros existentes
    for (int i = 0; i < proxRRN; i++)
    {
        Dados data;

        // Inicializa estrutura
        criar_dados(&data);

        // Lê um registro
        if (!ler_dados(&data, input_file))
            break;

        // Ignora registros removidos logicamente
        if (data.removido == '1')
        {
            liberar_dados(&data);
            continue;
        }

        // Verifica se o nome da estação coincide
        if (data.nomeEstacao && strcmp(data.nomeEstacao, nome_busca) == 0)
        {
            existe = 1; // Encontrou
            liberar_dados(&data);
            break;
        }

        liberar_dados(&data);
    }

    // Restaura a posição original do ponteiro
    fseek(input_file, pos_atual, SEEK_SET);

    return existe;
}

void BinarioNaTela(char* arquivo) {
    FILE* fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
            "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
            "não foi possível abrir o arquivo que me passou para leitura. "
            "Ele existe e você tá passando o nome certo? Você lembrou de "
            "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    unsigned char* mb = (unsigned char*)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++)
        cs += (unsigned long)mb[i];

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

void ScanQuoteString(char* str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ;

    if (R == 'N' || R == 'n') {
        getchar(); getchar(); getchar();
        strcpy(str, "");
    }
    else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1)
            strcpy(str, "");
        getchar();
    }
    else if (R != EOF) {
        str[0] = R;
        scanf("%s", &str[1]);
    }
    else {
        strcpy(str, "");
    }
}
