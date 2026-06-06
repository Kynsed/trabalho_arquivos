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
