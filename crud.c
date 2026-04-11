#include "crud.h"

void criarCabecalho(Cabecalho *cab) {
    cab->status = '1'; // arquivo consistente
    cab->topo = -1; // pilha de registros removidos vazia
    cab->proxRRN = 0; // próximo RRN a ser inserido
    cab->nroEstacoes = 0; // número de estações cadastradas
    cab->nroPares = 0; // número de pares de estações cadastrados
}

void criarDados(Dados *dados) {
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

void lerCsv() {
    char nomeCsv[100], nomeBin[100], buffer[100];
    scanf("%s %s", nomeCsv, nomeBin);

    FILE *csv = fopen(nomeCsv, "r");
    if (csv == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    FILE *bin = fopen(nomeBin, "wb");
    if (bin == NULL) {
        printf("Falha no processamento do arquivo.");
        fclose(csv);
        return;
    }

    Cabecalho cabecalho;
    criarCabecalho(&cabecalho);
    Dados novoDado;
    char **nomes = NULL; // variavel usada para armazenar os nomes das estações e facilitar a contagem do nroEstacoes

    // pular a primeira linha do CSV
    fseek(csv, 101, SEEK_CUR);

    // reservar espaco para o cabecalho
    fwrite("_________________", sizeof(char), 17, bin);

    while (1) {
        // inicializar os campos com os valores "nulos"
        criarDados(&novoDado);

        // INÍCIO DA SESSÃO DE LEITURA DOS CAMPOS

        lerInfo(csv, buffer);
        novoDado.codEstacao = atoi(buffer);

        novoDado.tamNomeEstacao = lerInfo(csv, buffer);
        novoDado.nomeEstacao = (char*)malloc((novoDado.tamNomeEstacao+1) * sizeof(char));
        strcpy(novoDado.nomeEstacao, buffer);
        nomes = (char **)realloc(nomes, (cabecalho.proxRRN + 1) * sizeof(char *));
        nomes[cabecalho.proxRRN] = (char*)malloc((novoDado.tamNomeEstacao+1) * sizeof(char));
        strcpy(nomes[cabecalho.proxRRN], buffer);
        if (novaEstacao(nomes, buffer, cabecalho.proxRRN)) {
            cabecalho.nroEstacoes++;
        }

        // a partir daqui vamos sempre checar se o campo é nulo

        if (lerInfo(csv, buffer) > 0) 
            novoDado.codLinha = atoi(buffer);

        novoDado.tamNomelinha = lerInfo(csv, buffer);
        if (novoDado.tamNomelinha > 0) {
            novoDado.nomeLinha = (char*)malloc((novoDado.tamNomelinha+1) * sizeof(char));
            strcpy(novoDado.nomeLinha, buffer);
        }

        if (lerInfo(csv, buffer) > 0) {
            novoDado.codProxEstacao = atoi(buffer);
            cabecalho.nroPares++; // se codProxEstacao não for nulo incrementamos o nroPares
        }

        if (lerInfo(csv, buffer) > 0)
            novoDado.distProxEstacao = atoi(buffer);

        if (lerInfo(csv, buffer) > 0)
            novoDado.codLinhaIntegra = atoi(buffer);

        if (lerInfo(csv, buffer) > 0)
            novoDado.codEstIntegra = atoi(buffer);

        cabecalho.proxRRN++;

        // FIM DA LEITURA DOS CAMPOS, INÍCIO DA SESSÃO DE ESCRITA DO REGISTRO

        fwrite(&novoDado.removido, sizeof(char), 1, bin);
        fwrite(&novoDado.proximo, sizeof(int), 1, bin);
        fwrite(&novoDado.codEstacao, sizeof(int), 1, bin);
        fwrite(&novoDado.codLinha, sizeof(int), 1, bin);
        fwrite(&novoDado.codProxEstacao, sizeof(int), 1, bin);
        fwrite(&novoDado.distProxEstacao, sizeof(int), 1, bin);
        fwrite(&novoDado.codLinhaIntegra, sizeof(int), 1, bin);
        fwrite(&novoDado.codEstIntegra, sizeof(int), 1, bin);
        fwrite(&novoDado.tamNomeEstacao, sizeof(int), 1, bin);
        fwrite(novoDado.nomeEstacao, sizeof(char), novoDado.tamNomeEstacao, bin);
        fwrite(&novoDado.tamNomelinha, sizeof(int), 1, bin);
        fwrite(novoDado.nomeLinha, sizeof(char), novoDado.tamNomelinha, bin);
        for (int j = 0; j < 80 - 37 - novoDado.tamNomeEstacao - novoDado.tamNomelinha; j++) {
            fputc('$', bin);
        }

        free(novoDado.nomeEstacao);
        free(novoDado.nomeLinha);

        // ler a quebra de linha, se na verdade for EOF encerrar a leitura
        if (lerInfo(csv, buffer) == -1) break;
    }

    // retorna ao início do binário e insere o cabecalho
    fseek(bin, 0, SEEK_SET);
    fwrite(&cabecalho.status, sizeof(char), 1, bin);
    fwrite(&cabecalho.topo, sizeof(int), 1, bin);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, bin);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, bin);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, bin);

    fclose(csv);
    fclose(bin);

    BinarioNaTela(nomeBin);

    for (int i = 0; i < cabecalho.proxRRN; i++) {
        free(nomes[i]);
    }
    free(nomes);
}

// lê 1 campo do csv, retorna -1 se EOF, retorna o valor de strlen() caso contrário
int lerInfo(FILE *csv, char buffer[100]) {
    int i = 0;
    int c;

    while (1) {
        c = fgetc(csv);

        // condição para parar de ler o campo
        if (c == ',' || c == '\n' || c == EOF || c == '\r') {
            break;
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';

    if (c == EOF) {
        return -1;
    }

    return i;
}

int novaEstacao(char **nomes, const char *nome, int n) {
    for (int i = 0; i < n; i++) {
        if (strcmp(nomes[i], nome) == 0) {
            return 0;
        }
    }
    return 1;
}

void liberarVetorDados(struct _dados **vetorDados, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        free(vetorDados[i]->nomeEstacao);
        free(vetorDados[i]->nomeLinha);
        free(vetorDados[i]);
    }
    free(vetorDados);
}

void BinarioNaTela(char *arquivo) {
    FILE *fs;
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
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

void busca(char *arquivoEntrada, int qntBuscas) {
    FILE *input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb"))) 
    {
        printf("Falha no processamento do arquivo.");
        return;
    }

    Cabecalho cabecalho;

    fread(&cabecalho.status, sizeof(char), 1, input_file);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    fread(&cabecalho.topo, sizeof(int), 1, input_file);
    fread(&cabecalho.proxRRN, sizeof(int), 1, input_file);
    fread(&cabecalho.nroEstacoes, sizeof(int), 1, input_file);
    fread(&cabecalho.nroPares, sizeof(int), 1, input_file);


    for (int i = 0; i < qntBuscas; i++) {
        int qntCampos;
        scanf(" %d", &qntCampos);

        char campo[50], valor[50];
        char vals[8][50] = {{0}};

        int existe = 0;
        for (int j = 0; j < qntCampos; j++) {
            scanf(" %s", campo);
            if (!strcmp(campo, "codEstacao")) {
                scanf(" %s", valor);
                strcpy(vals[0], valor);
            } else if (!strcmp(campo, "nomeEstacao")) {
                ScanQuoteString(valor);
                strcpy(vals[1], valor);
            } else if (!strcmp(campo, "codLinha")) {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[2], valor);
            } else if (!strcmp(campo, "nomeLinha")) {
                ScanQuoteString(valor);
                strcpy(vals[3], valor);
            } else if (!strcmp(campo, "codProxEstacao")) {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[4], valor);
            } else if (!strcmp(campo, "distProxEstacao")) {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[5], valor);
            } else if (!strcmp(campo, "codLinhaIntegra")) {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[6], valor);
            } else if (!strcmp(campo, "codEstIntegra")) {
                scanf(" %s", valor);
                if (strcmp(valor, "NULO") == 0)
                    strcpy(valor, "-1");
                strcpy(vals[7], valor);
            }
        }

        Dados *dados = (Dados *)malloc(sizeof(Dados));

        for (int j = 0; j < cabecalho.proxRRN; j++) {
            if (fread(&dados->removido, sizeof(char), 1, input_file) != 1)
                break;
            if (dados->removido == '1') {
                fseek(input_file, 79, SEEK_CUR);
                continue;
            }
            fread(&dados->proximo, sizeof(int), 1, input_file);
            fread(&dados->codEstacao, sizeof(int), 1, input_file);
            fread(&dados->codLinha, sizeof(int), 1, input_file);
            fread(&dados->codProxEstacao, sizeof(int), 1, input_file);
            fread(&dados->distProxEstacao, sizeof(int), 1, input_file);
            fread(&dados->codLinhaIntegra, sizeof(int), 1, input_file);
            fread(&dados->codEstIntegra, sizeof(int), 1, input_file);
            fread(&dados->tamNomeEstacao, sizeof(int), 1, input_file);

            if (dados->tamNomeEstacao > 0) {
                dados->nomeEstacao = malloc(dados->tamNomeEstacao + 1);
                fread(dados->nomeEstacao, sizeof(char), dados->tamNomeEstacao, input_file);
                dados->nomeEstacao[dados->tamNomeEstacao] = '\0';
            } else {
                dados->nomeEstacao = NULL;
            }

            fread(&dados->tamNomelinha, sizeof(int), 1, input_file);

            if (dados->tamNomelinha > 0) {
                dados->nomeLinha = malloc(dados->tamNomelinha + 1);
                fread(dados->nomeLinha, sizeof(char), dados->tamNomelinha, input_file);
                dados->nomeLinha[dados->tamNomelinha] = '\0';
            } else {
                dados->nomeLinha = NULL;
            }

            fseek(input_file, 80 - 37 - dados->tamNomeEstacao - dados->tamNomelinha, SEEK_CUR);

            if ((vals[0][0] == 0 || atoi(vals[0]) == dados->codEstacao) &&
                (vals[1][0] == 0 || !strcmp(vals[1], dados->nomeEstacao)) &&
                (vals[2][0] == 0 || atoi(vals[2]) == dados->codLinha) &&
                (vals[3][0] == 0 || (!dados->nomeLinha && !strcmp(vals[3], "NULO")) || !strcmp(vals[3], dados->nomeLinha)) &&
                (vals[4][0] == 0 || atoi(vals[4]) == dados->codProxEstacao) &&
                (vals[5][0] == 0 || atoi(vals[5]) == dados->distProxEstacao) &&
                (vals[6][0] == 0 || atoi(vals[6]) == dados->codLinhaIntegra) &&
                (vals[7][0] == 0 || atoi(vals[7]) == dados->codEstIntegra)) {
                printDados(dados);
                printf("\n");
                existe = 1;
            }

            if (dados->nomeEstacao)
                free(dados->nomeEstacao);
            if (dados->nomeLinha)
                free(dados->nomeLinha);
        }
        if (!existe) {
            printf("Registro inexistente.\n");
        }
        free(dados);
        printf("\n");

        if (i < qntBuscas - 1) {
            fseek(input_file, 17, SEEK_SET);
        }
    }

    fclose(input_file);
}

int header_reader(Cabecalho *cab, FILE *input_file)
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

int data_reader(Dados *data, FILE *input_file)
{
    if (data == NULL || input_file == NULL)
        return 0;

    unsigned char buffer[80];

    if (fread(buffer, 80, 1, input_file) != 1)
        return 0; // EOF

    int offset = 0;

    memcpy(&data->removido, buffer + offset, sizeof(char));
    offset += sizeof(char);

    memcpy(&data->proximo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->codEstacao, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->codLinha, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->codProxEstacao, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->distProxEstacao, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->codLinhaIntegra, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->codEstIntegra, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&data->tamNomeEstacao, buffer + offset, sizeof(int));
    offset += sizeof(int);

    if (data->tamNomeEstacao > 0)
    {
        data->nomeEstacao = malloc(data->tamNomeEstacao + 1);
        memcpy(data->nomeEstacao, buffer + offset, data->tamNomeEstacao);
        data->nomeEstacao[data->tamNomeEstacao] = '\0';
    }
    else
    {
        data->nomeEstacao = NULL;
    }

    offset += data->tamNomeEstacao;

    memcpy(&data->tamNomelinha, buffer + offset, sizeof(int));
    offset += sizeof(int);

    if (data->tamNomelinha > 0)
    {
        data->nomeLinha = malloc(data->tamNomelinha + 1);
        memcpy(data->nomeLinha, buffer + offset, data->tamNomelinha);
        data->nomeLinha[data->tamNomelinha] = '\0';
    }
    else
    {
        data->nomeLinha = NULL;
    }

    return 1;
}
/**
 *Recupera e exibe todos os registros válidos de um arquivo binário.
 *
 * Esta função implementa a funcionalidade de leitura completa do arquivo,
 * percorrendo todos os registros de dados e exibindo seus campos na saída padrão.
 *
 * Regras aplicadas:
 * - Ignora registros logicamente removidos (removido == '1')
 * - Trata campos nulos:
 *      - Inteiros nulos (-1) → "NULO"
 *      - Strings nulas (tamanho 0) → "NULO"
 * - Ignora bytes de lixo ('$') no final do registro
 * - Cada registro é exibido em uma linha
 */
void select_from(char *arquivoEntrada)
{
    FILE *input_file;

    // Validação do arquivo
    // Verifica se o nome do arquivo é válido e se foi possível abri-lo.
    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb"))) 
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    // Leitura e validação do cabeçalho
    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    int found = 0; // indica se algum registro válido foi encontrado

    //Loop principal de leitura até atingir EOF
    while (1)
    {
        Dados data;

        // Leitura do registro completo (80 bytes) usando função auxiliar
        if (!data_reader(&data, input_file))
            break;

        //Ignora registros logicamente removidos
        if (data.removido == '1')
        {
            if (data.nomeEstacao) 
                free(data.nomeEstacao);
            
            if (data.nomeLinha) 
                free(data.nomeLinha);
            
            continue;
        }

        found = 1;

        //Impressão dos campos (ordem definida pelo trabalho)
        printf("%d ", data.codEstacao);

        printf("%s ", data.nomeEstacao);

        (data.codLinha == -1) ? printf("NULO ") : printf("%d ", data.codLinha);

        (data.tamNomelinha == 0) ? printf("NULO ") : printf("%s ", data.nomeLinha);

        (data.codProxEstacao == -1) ? printf("NULO ") : printf("%d ", data.codProxEstacao);

        (data.distProxEstacao == -1) ? printf("NULO ") : printf("%d ", data.distProxEstacao);

        (data.codLinhaIntegra == -1) ? printf("NULO ") : printf("%d ", data.codLinhaIntegra);

        (data.codEstIntegra == -1) ? printf("NULO") : printf("%d", data.codEstIntegra);

        printf("\n");

        //Liberação de memória das strings
        if (data.nomeEstacao) 
            free(data.nomeEstacao);

        if (data.nomeLinha) 
            free(data.nomeLinha);
    }

    //Caso nenhum registro válido seja encontrado
    if (!found)
        printf("Registro inexistente.\n");

    fclose(input_file);
}

int tem_estacao_ativa(FILE *input_file, int proxRRN, const char *nome)
{
    const char *nome_busca = nome ? nome : "";

    long pos_atual = ftell(input_file); // salva posição original

    int existe = 0;

    // vai para o início dos registros
    fseek(input_file, 17, SEEK_SET);

    for (int i = 0; i < proxRRN; i++)
    {
        unsigned char buffer[80];

        if (fread(buffer, 80, 1, input_file) != 1)
            break;

        int offset = 0;

        char removido;
        
        memcpy(&removido, buffer + offset, sizeof(char));
        offset += sizeof(char);

        if (removido == '1')
            continue;

        // pula até tamNomeEstacao
        offset += sizeof(int) * 6; // proximo + codEstacao + codLinha + codProxEstacao + dist + codLinhaIntegra
        offset += sizeof(int);     // codEstIntegra

        int tamNomeEstacao;
        
        memcpy(&tamNomeEstacao, buffer + offset, sizeof(int));
        offset += sizeof(int);

        char nomeEstacao[100] = {0};

        if (tamNomeEstacao > 0 && tamNomeEstacao < 100)
        {
            memcpy(nomeEstacao, buffer + offset, tamNomeEstacao);
            nomeEstacao[tamNomeEstacao] = '\0';
        }

        if (strcmp(nomeEstacao, nome_busca) == 0)
        {
            existe = 1;
            break;
        }
    }

    // restaura posição original
    fseek(input_file, pos_atual, SEEK_SET);

    return existe;
}

void delete_from(char *arquivoEntrada)
{
    FILE *input_file;

    // Abre o arquivo para leitura e escrita binária
    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb+"))) 
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    // Leitura do cabeçalho usando função auxiliar
    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    // Marca o arquivo como inconsistente
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

        // Leitura dos filtros de busca
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

        // posiciona no primeiro registro
        fseek(input_file, 17, SEEK_SET);

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long pos = ftell(input_file);

            Dados data;

            // leitura do registro (80 bytes)
            if (!data_reader(&data, input_file))
                break;

            if (data.removido == '1') 
            {
                if (data.nomeEstacao) free(data.nomeEstacao);
                if (data.nomeLinha) free(data.nomeLinha);
                continue;
            }

            // Encontrou um match e vai removê-lo
            if (match_registro(&data, vals))
            {
                fseek(input_file, pos, SEEK_SET);

                char removido = '1';
                fwrite(&removido, sizeof(char), 1, input_file);

                fwrite(&cabecalho.topo, sizeof(int), 1, input_file);

                cabecalho.topo = rrn;

                // Atualiza nroPares
                if (data.codProxEstacao != -1)
                    cabecalho.nroPares--;

                // Atualiza nroEstacoes (verificação externa)
                if (!tem_estacao_ativa(input_file, cabecalho.proxRRN, data.nomeEstacao))
                    cabecalho.nroEstacoes--;

                // volta para posição correta de leitura
                fseek(input_file, pos + 80, SEEK_SET);
            }

            if (data.nomeEstacao) free(data.nomeEstacao);
            if (data.nomeLinha) free(data.nomeLinha);
        }
    }

    // Regrava o cabeçalho atualizado
    status = '1';

    fseek(input_file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, input_file);
    fwrite(&cabecalho.topo, sizeof(int), 1, input_file);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, input_file);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, input_file);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, input_file);

    fclose(input_file);

    BinarioNaTela(arquivoEntrada);
}

int match_registro(Dados *dados, char vals[8][50])
{
    return (vals[0][0] == 0 || atoi(vals[0]) == dados->codEstacao) && (vals[1][0] == 0 || 
            (dados->nomeEstacao && !strcmp(vals[1], dados->nomeEstacao))) &&

           (vals[2][0] == 0 || atoi(vals[2]) == dados->codLinha) &&

           (vals[3][0] == 0 || 
            (!dados->nomeLinha && !strcmp(vals[3], "NULO")) ||
            (dados->nomeLinha && !strcmp(vals[3], dados->nomeLinha))) &&

           (vals[4][0] == 0 || atoi(vals[4]) == dados->codProxEstacao) &&

           (vals[5][0] == 0 || atoi(vals[5]) == dados->distProxEstacao) &&

           (vals[6][0] == 0 || atoi(vals[6]) == dados->codLinhaIntegra) &&

           (vals[7][0] == 0 || atoi(vals[7]) == dados->codEstIntegra);
}

void update(char *arquivoEntrada)
{
    FILE* input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "rb+"))) 
    {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Cabecalho cabecalho;

    // leitura do cabeçalho usando função auxiliar
    if (!header_reader(&cabecalho, input_file))
    {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    // marca inconsistente
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

        fseek(input_file, 17, SEEK_SET);

        for (int rrn = 0; rrn < cabecalho.proxRRN; rrn++)
        {
            long pos = ftell(input_file);

            Dados data;

            // leitura do registro (80 bytes)
            if (!data_reader(&data, input_file))
                break;

            if (data.removido == '1') continue;
            if (!match_registro(&data, vals)) continue;

            char nomeEstacao[100] = {0};
            char nomeLinha[100] = {0};

            if (data.tamNomeEstacao > 0)
                strcpy(nomeEstacao, data.nomeEstacao);

            if (data.tamNomelinha > 0)
                strcpy(nomeLinha, data.nomeLinha);

            // =========================
            // APLICA UPDATE
            // =========================
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
                    } else 
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
                    } else 
                    {
                        data.tamNomelinha = strlen(valores[k]);
                        strcpy(nomeLinha, valores[k]);
                    }
                }
            }

            int novo_tam = 37 + data.tamNomeEstacao + data.tamNomelinha;

            if (novo_tam <= 80)
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

                int preenchido = novo_tam;
                while (preenchido < 80)
                {
                    fputc('$', input_file);
                    preenchido++;
                }
            }
        }
    }

    fseek(input_file, 1, SEEK_SET);
    fwrite(&cabecalho.topo, sizeof(int), 1, input_file);

    status = '1';
    fseek(input_file, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, input_file);

    fclose(input_file);

    BinarioNaTela(arquivoEntrada);
}

void printDados(Dados *data) {

    // print dos dados

        // codEstacao
        printf("%d ", data->codEstacao);

        // nomeEstacao
        printf("%s ", data->nomeEstacao);

        // codLinha
        (data->codLinha == -1) ? printf("NULO ") : printf("%d ", data->codLinha);

        // nomeLinha
        (data->tamNomelinha == 0) ? printf("NULO ") : printf("%s ", data->nomeLinha);

        // codProxEstacao
        (data->codProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->codProxEstacao);

        // distProxEstacao
        (data->distProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->distProxEstacao);

        // codLinhaIntegra
        (data->codLinhaIntegra == -1) ? printf("NULO ") : printf("%d ", data->codLinhaIntegra);

        // codEstIntegra
        (data->codEstIntegra == -1) ? printf("NULO") : printf("%d", data->codEstIntegra);
}

void inserir(char *arquivoEntrada, int qntInsercoes) {
    FILE *input_file;

    if (arquivoEntrada == NULL || !(input_file = fopen(arquivoEntrada, "r+b"))) 
    {
        printf("Falha no processamento do arquivo.");
        return;
    }

    Cabecalho cabecalho;

    fread(&cabecalho.status, sizeof(char), 1, input_file);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(input_file);
        return;
    }

    fread(&cabecalho.topo, sizeof(int), 1, input_file);
    fread(&cabecalho.proxRRN, sizeof(int), 1, input_file);
    fread(&cabecalho.nroEstacoes, sizeof(int), 1, input_file);
    fread(&cabecalho.nroPares, sizeof(int), 1, input_file);

    for (int i = 0; i < qntInsercoes; i++) {
        Dados novoDado;
        criarDados(&novoDado);

        char valor[50];

        scanf(" %s", valor);
        novoDado.codEstacao = atoi(valor);

        ScanQuoteString(valor);
        if (!strcmp(valor, "NULO")) {
            printf("Falha no processamento do arquivo.\n");
            break;
        } else {
            novoDado.tamNomeEstacao = strlen(valor);
            novoDado.nomeEstacao = (char*)malloc((strlen(valor)+1) * sizeof(char));
            strcpy(novoDado.nomeEstacao, valor);

            fseek(input_file, 46, SEEK_SET);
            int existe = 0;
            for (int j = 0; j < cabecalho.proxRRN; j++) {
                int tamanho;
                fread(&tamanho, sizeof(int), 1, input_file);
                char nome[tamanho + 1];
                fread(nome, sizeof(char), tamanho, input_file);
                nome[tamanho] = '\0';

                if (strcmp(nome, valor) == 0) {
                    existe = 1;
                    break;
                } else {
                    fseek(input_file, 80 - 4 - tamanho, SEEK_CUR);
                }
            }
            if (!existe) {
                cabecalho.nroEstacoes++;
            }
        }

        scanf(" %s", valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.codLinha = -1;
        } else {
            novoDado.codLinha = atoi(valor);
        }

        ScanQuoteString(valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.tamNomelinha = 0;
            novoDado.nomeLinha = NULL;
        } else {
            novoDado.tamNomelinha = strlen(valor);
            novoDado.nomeLinha = (char*)malloc((strlen(valor)+1) * sizeof(char));
            strcpy(novoDado.nomeLinha, valor);
        }

        scanf(" %s", valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.codProxEstacao = -1;
        } else {
            novoDado.codProxEstacao = atoi(valor);
            cabecalho.nroPares++;
        }

        scanf(" %s", valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.distProxEstacao = -1;
        } else {
            novoDado.distProxEstacao = atoi(valor);
        }

        scanf(" %s", valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.codLinhaIntegra = -1;
        } else {
            novoDado.codLinhaIntegra = atoi(valor);
        }

        scanf(" %s", valor);
        if (!strcmp(valor, "NULO")) {
            novoDado.codEstIntegra = -1;
        } else {
            novoDado.codEstIntegra = atoi(valor);
        }

        if (cabecalho.topo == -1) {
            fseek(input_file, 0, SEEK_END);
            cabecalho.proxRRN++;
        } else {
            fseek(input_file, 18 + 80*cabecalho.topo, SEEK_SET);
            int proximo;
            fread(&proximo, sizeof(int), 1, input_file);
            cabecalho.topo = proximo;
            fseek(input_file, -5, SEEK_CUR);
        }

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
        for (int j = 0; j < 80 - 37 - novoDado.tamNomeEstacao - novoDado.tamNomelinha; j++) {
            fputc('$', input_file); 
        }

        if (novoDado.nomeEstacao)
            free(novoDado.nomeEstacao);
        if (novoDado.nomeLinha)
            free(novoDado.nomeLinha);
    }
    fseek(input_file, 1, SEEK_SET);
    fwrite(&cabecalho.topo, sizeof(int), 1, input_file);
    fwrite(&cabecalho.proxRRN, sizeof(int), 1, input_file);
    fwrite(&cabecalho.nroEstacoes, sizeof(int), 1, input_file);
    fwrite(&cabecalho.nroPares, sizeof(int), 1, input_file);

    fclose(input_file);

    BinarioNaTela(arquivoEntrada);
}