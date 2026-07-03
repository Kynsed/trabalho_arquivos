#include "utilitarios.h"

/* Le um unico campo do CSV (delimitado por virgula/quebra de linha/EOF). */
char *lerInfo(FILE *csv)
{
    char buffer[100];
    int i = 0;
    int c;

    /* Acumula caracteres ate encontrar um delimitador de campo ou o fim. */
    while (1)
    {
        c = fgetc(csv);
        if (c == ',' || c == '\n' || c == EOF || c == '\r')
            break;
        buffer[i++] = c;
    }

    /* EOF sem conteudo sinaliza fim do arquivo para o chamador. */
    if (c == EOF)
        return NULL;

    buffer[i] = '\0';

    char *info = (char *)malloc((i + 1) * sizeof(char));
    strcpy(info, buffer);
    return info;
}

/* Le da entrada padrao uma string possivelmente entre aspas ou a palavra NULO. */
void ScanQuoteString(char *str)
{
    char R;

    /* Ignora espacos em branco iniciais (espaco, \r, \n...). */
    while ((R = getchar()) != EOF && isspace(R))
        ;

    if (R == 'N' || R == 'n')
    {
        /* Campo NULO: consome o restante de "ULO" e devolve string vazia. */
        getchar();
        getchar();
        getchar();
        strcpy(str, "");
    }
    else if (R == '\"')
    {
        /* Le tudo ate a aspa de fechamento. */
        if (scanf("%[^\"]", str) != 1)
            strcpy(str, "");
        getchar(); /* descarta a aspa de fechamento */
    }
    else if (R != EOF)
    {
        /* Valor sem aspas: le como token simples a partir do 1o caractere. */
        str[0] = R;
        scanf("%s", &str[1]);
    }
    else
    {
        strcpy(str, "");
    }
}

/* Utilitario do trabalho: imprime um checksum normalizado do arquivo binario. */
void BinarioNaTela(char *arquivo)
{
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb")))
    {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    /* Descobre o tamanho total do arquivo. */
    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    /* Carrega o arquivo inteiro em memoria para somar os bytes. */
    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++)
        cs += (unsigned long)mb[i];

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

int tem_estacao_ativa(FILE *input_file, int proxRRN, const char *nome)
{
    const char *nome_busca = nome ? nome : "";

    long pos_atual = ftell(input_file); /* salva posicao original */
    int existe = 0;

    /* Vai para o inicio da area de registros (logo apos o cabecalho). */
    fseek(input_file, TAM_CABECALHO, SEEK_SET);

    for (int i = 0; i < proxRRN; i++)
    {
        unsigned char buffer[TAM_REGISTRO];

        if (fread(buffer, TAM_REGISTRO, 1, input_file) != 1)
            break;

        int offset = 0;
        char removido;

        memcpy(&removido, buffer + offset, sizeof(char));
        offset += sizeof(char);

        /* Pula registros removidos assim que o status removido e lido como '1'. */
        if (removido == '1')
            continue;

        /* Avanca ate o campo tamNomeEstacao (7 inteiros + codEstIntegra). */
        offset += sizeof(int) * 6;
        offset += sizeof(int);

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

    fseek(input_file, pos_atual, SEEK_SET); /* restaura posicao */
    return existe;
}

int compE(const void *a, const void *b) {
    Dados *d1 = (Dados *)a;
    Dados *d2 = (Dados *)b;

    return d1->codEstacao - d2->codEstacao;
}

int compPE(const void *a, const void *b) {
    Dados *d1 = (Dados *)a;
    Dados *d2 = (Dados *)b;

    return d1->codProxEstacao - d2->codProxEstacao;
}
