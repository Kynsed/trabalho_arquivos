#include "registro.h"

/* Inicializa um registro vazio com valores neutros. */
struct _dados *criarDados()
{
    struct _dados *dados = (struct _dados *)malloc(sizeof(struct _dados));
    if (dados == NULL)
        return NULL;

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

/* Le e desserializa um registro de 80 bytes do disco. */
int data_reader(Dados *data, FILE *input_file)
{
    if (data == NULL || input_file == NULL)
        return 0;

    /* Le o registro inteiro de uma vez para um buffer; assim a leitura de
     * disco e feita em um unico acesso, e a desserializacao ocorre em memoria. */
    unsigned char buffer[TAM_REGISTRO];

    if (fread(buffer, TAM_REGISTRO, 1, input_file) != 1)
        return 0; /* EOF ou erro */

    int offset = 0;

    /* Campos de tamanho fixo, copiados na ordem em que foram gravados. */
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

    /* Campo de tamanho variavel: nomeEstacao. */
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

    /* Campo de tamanho variavel: nomeLinha. */
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

/* Libera o vetor de registros mantido em memoria durante a carga do CSV. */
void liberarVetorDados(struct _dados **vetorDados, int tamanho)
{
    for (int i = 0; i < tamanho; i++)
    {
        free(vetorDados[i]->nomeEstacao);
        free(vetorDados[i]->nomeLinha);
        free(vetorDados[i]);
    }
    free(vetorDados);
}

/* Imprime um registro, traduzindo campos ausentes para "NULO". */
void printDados(Dados *data)
{
    /* codEstacao (chave primaria, sempre presente). */
    printf("%d ", data->codEstacao);

    /* nomeEstacao (sempre presente). */
    printf("%s ", data->nomeEstacao);

    /* Demais campos podem ser nulos (-1 para inteiros, tamanho 0 para strings). */
    (data->codLinha == -1) ? printf("NULO ") : printf("%d ", data->codLinha);

    (data->tamNomelinha == 0) ? printf("NULO ") : printf("%s ", data->nomeLinha);

    (data->codProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->codProxEstacao);

    (data->distProxEstacao == -1) ? printf("NULO ") : printf("%d ", data->distProxEstacao);

    (data->codLinhaIntegra == -1) ? printf("NULO ") : printf("%d ", data->codLinhaIntegra);

    (data->codEstIntegra == -1) ? printf("NULO") : printf("%d", data->codEstIntegra);
}

/* Testa se um registro casa com todos os criterios de busca informados. */
int match_registro(Dados *dados, char vals[8][50])
{
    /* Para cada campo: ou nao foi usado como filtro (string vazia),
     * ou o valor do registro precisa coincidir com o filtro. */
    return (vals[0][0] == 0 || atoi(vals[0]) == dados->codEstacao) &&

           (vals[1][0] == 0 ||
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
