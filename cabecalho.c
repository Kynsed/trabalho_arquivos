#include "cabecalho.h"

/* Cria o cabecalho em memoria ja marcado como inconsistente ('0'). */
Cabecalho *criarCabecalho()
{
    Cabecalho *cabecalho = (Cabecalho *)malloc(sizeof(Cabecalho));
    if (cabecalho == NULL)
        return NULL;

    cabecalho->status = '0';   /* inconsistente ate o fim da gravacao */
    cabecalho->topo = -1;      /* pilha de removidos vazia */
    cabecalho->proxRRN = 0;    /* nenhum registro inserido ainda */
    cabecalho->nroEstacoes = 0;
    cabecalho->nroPares = 0;

    return cabecalho;
}

/* Le o cabecalho do disco e valida a consistencia do arquivo. */
int header_reader(Cabecalho *cab, FILE *input_file)
{
    if (cab == NULL || input_file == NULL)
        return 0;

    /* Cada fread precisa ler exatamente 1 elemento; qualquer falha invalida o cabecalho. */
    if (fread(&cab->status, sizeof(char), 1, input_file) != 1 ||
        fread(&cab->topo, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->proxRRN, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->nroEstacoes, sizeof(int), 1, input_file) != 1 ||
        fread(&cab->nroPares, sizeof(int), 1, input_file) != 1)
        return 0;

    /* Arquivo so e utilizavel se estiver consistente. */
    return (cab->status == '1');
}

/* Grava o cabecalho no disco na ordem fixa dos campos. */
void escreverCabecalho(Cabecalho *cab, FILE *output_file)
{
    if (cab == NULL || output_file == NULL)
        return;

    /* Gravacao direta em disco: ordem identica a usada na leitura. */
    fwrite(&cab->status, sizeof(char), 1, output_file);
    fwrite(&cab->topo, sizeof(int), 1, output_file);
    fwrite(&cab->proxRRN, sizeof(int), 1, output_file);
    fwrite(&cab->nroEstacoes, sizeof(int), 1, output_file);
    fwrite(&cab->nroPares, sizeof(int), 1, output_file);
}
