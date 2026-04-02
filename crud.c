#include "crud.h"

// struct _cabecalho {
//     char status; // 0 ou 1, indicando se o arquivo está consistente ou inconsistente
//     int topo; // byte offset do topo da pilha de registros removidos
//     int proxRRN; // próximo RRN a ser inserido
//     int nroEstacoes; // número de estações cadastradas
//     int nroPares; // número de pares de estações cadastrados
// };
    
// struct _dados {
//     char removido;
//     int proximo;
//     int codEstacao;
//     int codLinha;
//     int codProxEstacao;
//     int distProxEstacao;
//     int codLinhaIntegra;
//     int codEstIntegra;
//     int tamNomeEstacao;
//     char *nomeEstacao;
//     int tamNomelinha;
//     char *nomeLinha;
// };

struct _cabecalho *criarCabecalho() {
    struct _cabecalho* cabecalho = (struct _cabecalho*)malloc(sizeof(struct _cabecalho));
    cabecalho->status = '1'; // arquivo consistente
    cabecalho->topo = -1; // pilha de registros removidos vazia
    cabecalho->proxRRN = 0; // próximo RRN a ser inserido
    cabecalho->nroEstacoes = 0; // número de estações cadastradas
    cabecalho->nroPares = 0; // número de pares de estações cadastrados

    return cabecalho;
}

struct _dados *criarDados() {
    struct _dados* dados = (struct _dados*)malloc(sizeof(struct _dados));
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

    return dados;
}

void lerCsv() {
    char nomeCsv[100], nomeBin[100], *info, skip[102];
    scanf("%s %s", nomeCsv, nomeBin);

    FILE *csv = fopen(nomeCsv, "r");
    if (csv == NULL) {
        fprintf(stderr, "Falha no processamento do arquivo.");
        return;
    }

    FILE *bin = fopen(nomeBin, "wb");
    if (bin == NULL) {
        fprintf(stderr, "Falha no processamento do arquivo.");
        fclose(csv);
        return;
    }

    struct _cabecalho *cabecalho = criarCabecalho();
    struct _dados **vetorDados = NULL;
    int c;

    fgets(skip, 102, csv); // ler e ignorar a primeira linha do CSV (cabeçalho)

    while (1) {
        info = lerInfo(csv);
        if (info == NULL) break;
        struct _dados *novoDado = criarDados();

        novoDado->codEstacao = atoi(info);
        free(info);

        info = lerInfo(csv);
        novoDado->tamNomeEstacao = strlen(info);
        novoDado->nomeEstacao = (char*)malloc((strlen(info)+1) * sizeof(char));
        strcpy(novoDado->nomeEstacao, info);
        if (novaEstacao(vetorDados, cabecalho->proxRRN, info)) {
            cabecalho->nroEstacoes++;
        }
        free(info);

        info = lerInfo(csv);
        if (strlen(info) > 0) 
            novoDado->codLinha = atoi(info);
        free(info);

        info = lerInfo(csv);
        if (strlen(info) > 0) {
            novoDado->tamNomelinha = strlen(info);
            novoDado->nomeLinha = (char*)malloc((strlen(info)+1) * sizeof(char));
            strcpy(novoDado->nomeLinha, info);
        }
        free(info);

        info = lerInfo(csv);
        if (strlen(info) > 0) {
            novoDado->codProxEstacao = atoi(info);
            cabecalho->nroPares++;
        }
        free(info);

        info = lerInfo(csv);
        if (strlen(info) > 0)
            novoDado->distProxEstacao = atoi(info);
        free(info);

        info = lerInfo(csv);
        if (strlen(info) > 0)
        novoDado->codLinhaIntegra = atoi(info);
        free(info);

        info = lerInfo(csv);
        if (info &&strlen(info) > 0)
            novoDado->codEstIntegra = atoi(info);
        free(info);

        info = lerInfo(csv);
        free(info);
        
        cabecalho->proxRRN++;
        vetorDados = (struct _dados**)realloc(vetorDados, cabecalho->proxRRN * sizeof(struct _dados*));
        vetorDados[cabecalho->proxRRN - 1] = novoDado;

        if (info==NULL) break;
    }

    fwrite(&cabecalho->status, sizeof(char), 1, bin);
    fwrite(&cabecalho->topo, sizeof(int), 1, bin);
    fwrite(&cabecalho->proxRRN, sizeof(int), 1, bin);
    fwrite(&cabecalho->nroEstacoes, sizeof(int), 1, bin);
    fwrite(&cabecalho->nroPares, sizeof(int), 1, bin);
    
    for (int i = 0; i < cabecalho->proxRRN; i++) {
        long pos = ftell(bin);
        fwrite(&vetorDados[i]->removido, sizeof(char), 1, bin);
        fwrite(&vetorDados[i]->proximo, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codLinha, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codProxEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->distProxEstacao, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codLinhaIntegra, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->codEstIntegra, sizeof(int), 1, bin);
        fwrite(&vetorDados[i]->tamNomeEstacao, sizeof(int), 1, bin);
        fwrite(vetorDados[i]->nomeEstacao, sizeof(char), vetorDados[i]->tamNomeEstacao, bin);
        fwrite(&vetorDados[i]->tamNomelinha, sizeof(int), 1, bin);
        fwrite(vetorDados[i]->nomeLinha, sizeof(char), vetorDados[i]->tamNomelinha, bin);
        long pad = ftell(bin) - pos;
        while (pad < 80) {
            fputc('$', bin);
            pad++;
        }
    }

    fclose(csv);
    fclose(bin);

    BinarioNaTela(nomeBin);

    liberarVetorDados(vetorDados, cabecalho->proxRRN);
    free(cabecalho);
}

char *lerInfo(FILE *csv) {
    char buffer[100];
    int i = 0;
    int c;

    while (1) {
        c = fgetc(csv);
        if (c == ',' || c == '\n' || c == EOF || c == '\r') {
            break;
        }
        buffer[i++] = c;
    }

    if (c == EOF) {
        return NULL;
    }

    buffer[i] = '\0';

    char *info = (char *)malloc((i + 1) * sizeof(char));
    strcpy(info, buffer);
    return info;
}

int novaEstacao(struct _dados **vetorDados, int n, const char *nome) {
    for (int i = 0; i < n; i++) {
        if (strcmp(vetorDados[i]->nomeEstacao, nome) == 0) {
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
