#include "arvoreB.h"

/* ========================================================================
 *  Cabecalho do indice
 * ====================================================================== */

/* Cabecalho de arvore vazia: inconsistente ate o fim da escrita. */
void cabecalhoArvore_inicializar(CabecalhoArvoreB *cab)
{
    cab->status = '0';
    cab->noRaiz = -1;
    cab->topo = -1;
    cab->proxRRN = 0;
    cab->nroNos = 0;
}

/*
 * Inicializa o cabecalho da árvore em memória para uma árvore vazia.
 * Comentário: Este procedimento seta o arquivo como inconsistente ('0')
 * até que todos os dados sejam gravados e o cabeçalho seja reescrito.
 */

/* Le o cabecalho do indice e verifica a consistencia. */
int cabecalhoArvore_ler(CabecalhoArvoreB *cab, FILE *arq)
{
    if (cab == NULL || arq == NULL)
        return 0;

    fseek(arq, 0, SEEK_SET);

    /* Le cada campo na ordem definida; qualquer falha invalida o cabecalho. */
    if (fread(&cab->status, sizeof(char), 1, arq) != 1 ||
        fread(&cab->noRaiz, sizeof(int), 1, arq) != 1 ||
        fread(&cab->topo, sizeof(int), 1, arq) != 1 ||
        fread(&cab->proxRRN, sizeof(int), 1, arq) != 1 ||
        fread(&cab->nroNos, sizeof(int), 1, arq) != 1)
        return 0;

    return (cab->status == '1');
}

/*
 * Lê o cabeçalho do arquivo de índice. Retorna 1 se o arquivo estiver
 * consistente ('1'), caso contrário retorna 0.
 */

/* Grava o cabecalho do indice campo a campo, na ordem da especificacao. */
void cabecalhoArvore_escrever(CabecalhoArvoreB *cab, FILE *arq)
{
    fseek(arq, 0, SEEK_SET);

    /* Escrita direta em disco: status | noRaiz | topo | proxRRN | nroNos. */
    fwrite(&cab->status, sizeof(char), 1, arq);
    fwrite(&cab->noRaiz, sizeof(int), 1, arq);
    fwrite(&cab->topo, sizeof(int), 1, arq);
    fwrite(&cab->proxRRN, sizeof(int), 1, arq);
    fwrite(&cab->nroNos, sizeof(int), 1, arq);
}

/* ========================================================================
 *  Nos do indice
 * ====================================================================== */

/* No vazio: ativo, folha, sem chaves; todos os campos de dados em -1. */
void noArvore_inicializar(NoArvoreB *no)
{
    no->removido = '0';
    no->proximo = -1;
    no->tipoNo = NO_FOLHA;
    no->nroChaves = 0;

    for (int i = 0; i < MAX_CHAVES; i++)
    {
        no->C[i] = -1;
        no->PR[i] = -1;
    }
    for (int i = 0; i < MAX_FILHOS; i++)
        no->P[i] = -1;
}

static void copia_no(NoArvoreB *no1, NoArvoreB *no2)
{
    no1->removido = no2->removido;
    no1->proximo = no2->proximo;
    no1->tipoNo = no2->tipoNo;
    no1->nroChaves = no2->nroChaves;

    for (int i = 0; i < MAX_CHAVES; i++)
    {
        no1->C[i] = no2->C[i];
        no1->PR[i] = no2->PR[i];
    }
    for (int i = 0; i < MAX_FILHOS; i++)
        no1->P[i] = no2->P[i];
}

/*
 * Inicializa um nó vazio na memória: marca como ativo, folha e zera todos
 * os campos de chaves e ponteiros com -1. Usado antes de gravar um novo nó
 * no disco ou ao construir nós temporários.
 */

/* Calcula o byte offset do no de RRN 'rrn' no arquivo de indice. */
static long offset_no(int rrn)
{
    return (long)TAM_CABECALHO_ARVORE + (long)TAM_NO_ARVORE * rrn;
}

/* Retorna o deslocamento (em bytes) do nó de RRN indicado no arquivo. */

#define MIN_CHAVES ((ORDEM_ARVORE + 1) / 2 - 1)

static int posicao_chave(NoArvoreB *no, int chave)
{
    for (int i = 0; i < no->nroChaves; i++)
    {
        if (no->C[i] == chave)
            return i;
    }
    return -1;
}

/*
 * Escolhe o índice do ponteiro para descer na busca/inserção.
 * Retorna i tal que todas chaves em P[i] < chave <= C[i] (conceitualmente).
 */

static void liberar_no(CabecalhoArvoreB *cabArvore, NoArvoreB *no, int rrn, FILE *arq)
{
    no->removido = '1';
    no->proximo = cabArvore->topo;
    fseek(arq, offset_no(rrn), SEEK_SET);
    fwrite(&no->removido, sizeof(char), 1, arq);
    fwrite(&no->proximo, sizeof(int), 1, arq);
    cabArvore->topo = rrn;
    cabArvore->nroNos--;
}

static int corrigir_underflow(FILE *arq, CabecalhoArvoreB *cabArvore,
                              NoArvoreB *noPai, int rrnPai, int indexFilho)
{
    int rrnFilho = noPai->P[indexFilho];
    NoArvoreB noFilho;
    noArvore_ler(&noFilho, rrnFilho, arq);

    // talvez seja inútil
    if (noFilho.nroChaves >= MIN_CHAVES) {
        return 0; /* No filho tem chaves suficientes: nada a corrigir. */
    }

    int rrnEsq = -1;
    if (indexFilho > 0)
        rrnEsq = noPai->P[indexFilho - 1];

    int rrnDir = -1;
    if (indexFilho < noPai->nroChaves)
        rrnDir = noPai->P[indexFilho + 1];

    /* Analise da quantidade de chaves dos nos irmaos*/
    NoArvoreB noEsq, noDir;
    if (rrnEsq != -1)
        noArvore_ler(&noEsq, rrnEsq, arq);
    if (rrnDir != -1)
        noArvore_ler(&noDir, rrnDir, arq);


    /* Redistribuicao começando com o irmao a direita*/
    if (rrnDir != -1 && noDir.nroChaves > MIN_CHAVES)
    {
        //while (noFilho.nroChaves < noDir.nroChaves && noDir.nroChaves > MIN_CHAVES)
        {
            noFilho.C[noFilho.nroChaves] = noPai->C[indexFilho];
            noFilho.PR[noFilho.nroChaves] = noPai->PR[indexFilho];
            noFilho.P[noFilho.nroChaves + 1] = noDir.P[0];
            noFilho.nroChaves++;

            noPai->C[indexFilho] = noDir.C[0];
            noPai->PR[indexFilho] = noDir.PR[0];

            for (int j = 0; j < noDir.nroChaves - 1; j++)
            {
                noDir.C[j] = noDir.C[j + 1];
                noDir.PR[j] = noDir.PR[j + 1];
                noDir.P[j] = noDir.P[j + 1];
            }
            noDir.P[noDir.nroChaves - 1] = noDir.P[noDir.nroChaves];
            noDir.P[noDir.nroChaves] = -1;
            noDir.C[noDir.nroChaves - 1] = -1;
            noDir.PR[noDir.nroChaves - 1] = -1;
            noDir.nroChaves--;
        }

        noArvore_escrever(&noDir, rrnDir, arq);
        noArvore_escrever(&noFilho, rrnFilho, arq);
        noArvore_escrever(noPai, rrnPai, arq);
        return 0;
    }

    /* Redistribuicao com irmao da esquerda */
    if (rrnEsq != -1 && noEsq.nroChaves > MIN_CHAVES)
    {
        //while (noEsq.nroChaves - noFilho.nroChaves > 1 && noEsq.nroChaves > MIN_CHAVES)
        {
            for (int j = noFilho.nroChaves; j > 0; j--)
            {
                noFilho.C[j] = noFilho.C[j - 1];
                noFilho.PR[j] = noFilho.PR[j - 1];
                noFilho.P[j + 1] = noFilho.P[j];
            }
            noFilho.P[1] = noFilho.P[0];

            noFilho.C[0] = noPai->C[indexFilho - 1];
            noFilho.PR[0] = noPai->PR[indexFilho - 1];
            noFilho.P[0] = noEsq.P[noEsq.nroChaves];
            noFilho.nroChaves++;

            noPai->C[indexFilho - 1] = noEsq.C[noEsq.nroChaves - 1];
            noPai->PR[indexFilho - 1] = noEsq.PR[noEsq.nroChaves - 1];
            noEsq.C[noEsq.nroChaves - 1] = -1;
            noEsq.PR[noEsq.nroChaves - 1] = -1;
            noEsq.P[noEsq.nroChaves] = -1;
            noEsq.nroChaves--;
        }

        noArvore_escrever(&noEsq, rrnEsq, arq);
        noArvore_escrever(&noFilho, rrnFilho, arq);
        noArvore_escrever(noPai, rrnPai, arq);
        return 0;
    }

    /* Concatenacao a esquerda primeiro */
    if (rrnEsq != -1)
    {
        noEsq.C[noEsq.nroChaves] = noPai->C[indexFilho - 1];
        noEsq.PR[noEsq.nroChaves] = noPai->PR[indexFilho - 1];

        for (int j = 0; j < noFilho.nroChaves; j++)
        {
            noEsq.C[noEsq.nroChaves + 1 + j] = noFilho.C[j];
            noEsq.PR[noEsq.nroChaves + 1 + j] = noFilho.PR[j];
            noEsq.P[noEsq.nroChaves + 1 + j] = noFilho.P[j];
        }
        noEsq.P[noEsq.nroChaves + noFilho.nroChaves + 1] = noFilho.P[noFilho.nroChaves];

        noEsq.nroChaves += 1 + noFilho.nroChaves;

        for (int j = indexFilho - 1; j < noPai->nroChaves - 1; j++)
        {
            noPai->C[j] = noPai->C[j + 1];
            noPai->PR[j] = noPai->PR[j + 1];
            noPai->P[j + 1] = noPai->P[j + 2];
        }
        noPai->C[noPai->nroChaves - 1] = -1;
        noPai->PR[noPai->nroChaves - 1] = -1;
        noPai->P[noPai->nroChaves] = -1;
        noPai->nroChaves--;

        noArvore_escrever(&noEsq, rrnEsq, arq);
        noArvore_escrever(noPai, rrnPai, arq);
        liberar_no(cabArvore, &noFilho, rrnFilho, arq);
        return (noPai->nroChaves < MIN_CHAVES);
    }

    /* Concatenacao com o no a direita*/
    else // if (rrnDir != -1) - else desnecessario, mas mantido para clareza
    {
        noFilho.C[noFilho.nroChaves] = noPai->C[indexFilho];
        noFilho.PR[noFilho.nroChaves] = noPai->PR[indexFilho];

        for (int j = 0; j < noDir.nroChaves; j++)
        {
            noFilho.C[noFilho.nroChaves + 1 + j] = noDir.C[j];
            noFilho.PR[noFilho.nroChaves + 1 + j] = noDir.PR[j];
            noFilho.P[noFilho.nroChaves + 1 + j] = noDir.P[j];
        }
        noFilho.P[noFilho.nroChaves + noDir.nroChaves + 1] = noDir.P[noDir.nroChaves];

        noFilho.nroChaves += 1 + noDir.nroChaves;

        for (int j = indexFilho; j < noPai->nroChaves - 1; j++)
        {
            noPai->C[j] = noPai->C[j + 1];
            noPai->PR[j] = noPai->PR[j + 1];
            noPai->P[j + 1] = noPai->P[j + 2];
        }
        noPai->C[noPai->nroChaves - 1] = -1;
        noPai->PR[noPai->nroChaves - 1] = -1;
        noPai->P[noPai->nroChaves] = -1;
        noPai->nroChaves--;


        noArvore_escrever(&noFilho, rrnFilho, arq);
        noArvore_escrever(noPai, rrnPai, arq);
        liberar_no(cabArvore, &noDir, rrnDir, arq);
        return (noPai->nroChaves < MIN_CHAVES);
    }
}

/* Essa rotina remove a chave e, quando necessário, sinaliza se ocorreu underflow em um filho 
 * (retornando 1 para forçar correção a partir do pai).*/
static int delete_chave_internal(FILE *arq, CabecalhoArvoreB *cab, int rrn, int chave)
{
    if (rrn == -1)
        return 0;

    NoArvoreB no;
    noArvore_ler(&no, rrn, arq);

    int pos = posicao_chave(&no, chave);
    if (no.tipoNo == NO_FOLHA)
    {
        if (pos == -1)
            return 0;

        /* shift das chaves*/
        for (int j = pos; j < no.nroChaves - 1; j++)
        {
            no.C[j] = no.C[j + 1];
            no.PR[j] = no.PR[j + 1];
        }
        no.C[no.nroChaves - 1] = -1;
        no.PR[no.nroChaves - 1] = -1;
        no.nroChaves--;

        noArvore_escrever(&no, rrn, arq);
        return (no.nroChaves < MIN_CHAVES);
    }

    int childIndex;
    int childUnderflow = 0;

    if (pos != -1)
    {
        /* busca o sucessor */
        int sucessorRrn = no.P[pos + 1];
        NoArvoreB sucessor;
        noArvore_ler(&sucessor, sucessorRrn, arq);
        while (sucessor.tipoNo != NO_FOLHA)
        {
            sucessorRrn = sucessor.P[0];
            noArvore_ler(&sucessor, sucessorRrn, arq);
        }

        no.C[pos] = sucessor.C[0];
        no.PR[pos] = sucessor.PR[0];
        noArvore_escrever(&no, rrn, arq);

        childUnderflow = delete_chave_internal(arq, cab, no.P[pos + 1], no.C[pos]);
        if (childUnderflow){
            return corrigir_underflow(arq, cab, &no, rrn, pos + 1);
        }
        return 0;
    }

    childIndex = 0;
    while (childIndex < no.nroChaves && chave > no.C[childIndex])
        childIndex++;
    childUnderflow = delete_chave_internal(arq, cab, no.P[childIndex], chave);
    if (childUnderflow) {
        return corrigir_underflow(arq, cab, &no, rrn, childIndex);
    }

    return 0;
}

/* Funcao principal para deletar uma chave na ArvoreB */
void delete_chave(FILE *arq, CabecalhoArvoreB *cab, int chave)
{
    delete_chave_internal(arq, cab, cab->noRaiz, chave);

    /* Recarrega a raiz para inspecionar seu estado após a remoção */
    NoArvoreB raiz;
    noArvore_ler(&raiz, cab->noRaiz, arq);

    /* Se a raiz ficou sem chaves, tratamos a redução de altura ou remoção */
    if (raiz.nroChaves == 0)
    {
        /*
         * Raiz com um filho: promove o filho para nova raiz (diminuicao da altura da arvore).
        */
        if (raiz.P[0] != -1)
        {
            int novoRaizRRN = raiz.P[0];
            NoArvoreB novoRaiz;

            /* Leitura do filho para determinar a tipagem*/
            noArvore_ler(&novoRaiz, novoRaizRRN, arq);
            novoRaiz.tipoNo = (novoRaiz.P[0] == -1) ? NO_FOLHA : NO_RAIZ;
            noArvore_escrever(&novoRaiz, novoRaizRRN, arq);
            liberar_no(cab, &raiz, cab->noRaiz, arq);
            cab->noRaiz = novoRaizRRN;
        }
        else
        {
            /* Raiz sem filhos e sem chaves: árvore fica vazia */
            liberar_no(cab, &raiz, cab->noRaiz, arq);
            cab->noRaiz = -1;
            cab->nroNos = 0;
        }

    }
    else if (raiz.P[0] != -1)
    {
        /* Ainda tem filhos: marca corretamente como raiz interna */
        raiz.tipoNo = NO_RAIZ;
        noArvore_escrever(&raiz, cab->noRaiz, arq);
    }
    else
    {
        /* Não há filhos: a raiz é uma folha */
        raiz.tipoNo = NO_FOLHA;
        noArvore_escrever(&raiz, cab->noRaiz, arq);
    }
}

/* ========================================================================
 *  Busca na arvore-B
 * ====================================================================== */

 /* Uma so chamada de I/O (1 fseek + 1 fread) reduz o custo de acesso. */
void noArvore_ler(NoArvoreB *no, int rrn, FILE *arq)
{
    unsigned char buf[TAM_NO_ARVORE]; /* buffer de pilha; sem heap */

    fseek(arq, offset_no(rrn), SEEK_SET);
    if (fread(buf, TAM_NO_ARVORE, 1, arq) != 1)
        return;

    int off = 0;

    memcpy(&no->removido, buf + off, sizeof(char)); off += sizeof(char);
    memcpy(&no->proximo, buf + off, sizeof(int));   off += sizeof(int);
    memcpy(&no->tipoNo, buf + off, sizeof(int));     off += sizeof(int);
    memcpy(&no->nroChaves, buf + off, sizeof(int));  off += sizeof(int);

    /* Chaves e referencias intercaladas: C1,PR1,C2,PR2,C3,PR3. */
    for (int i = 0; i < MAX_CHAVES; i++)
    {
        memcpy(&no->C[i], buf + off, sizeof(int));  off += sizeof(int);
        memcpy(&no->PR[i], buf + off, sizeof(int)); off += sizeof(int);
    }
    /* Ponteiros para subarvores: P1,P2,P3,P4. */
    for (int i = 0; i < MAX_FILHOS; i++)
    {
        memcpy(&no->P[i], buf + off, sizeof(int)); off += sizeof(int);
    }
}

/* Grava um no (53 bytes) no disco usando buffer para evitar padding da struct. */
void noArvore_escrever(NoArvoreB *no, int rrn, FILE *arq)
{
    unsigned char buf[TAM_NO_ARVORE];
    int off = 0;

    fseek(arq, offset_no(rrn), SEEK_SET);

    memcpy(buf + off, &no->removido, sizeof(char)); off += sizeof(char);
    memcpy(buf + off, &no->proximo, sizeof(int));   off += sizeof(int);
    memcpy(buf + off, &no->tipoNo, sizeof(int));     off += sizeof(int);
    memcpy(buf + off, &no->nroChaves, sizeof(int));  off += sizeof(int);

    /* Chaves e referencias intercaladas: C1,PR1,C2,PR2,C3,PR3. */
    for (int i = 0; i < MAX_CHAVES; i++)
    {
        memcpy(buf + off, &no->C[i], sizeof(int));  off += sizeof(int);
        memcpy(buf + off, &no->PR[i], sizeof(int)); off += sizeof(int);
    }
    /* Ponteiros para subarvores: P1,P2,P3,P4. */
    for (int i = 0; i < MAX_FILHOS; i++)
    {
        memcpy(buf + off, &no->P[i], sizeof(int)); off += sizeof(int);
    }

    fwrite(buf, TAM_NO_ARVORE, 1, arq);
}

/* ========================================================================
 *  Insercao na arvore-B (com split)
 * ====================================================================== */

/*
 * aloca_no: obtem o RRN de um no livre para escrita. Reaproveita o topo da
 * pilha de nos logicamente removidos quando houver (topo != -1); caso
 * contrario, usa um novo RRN ao final do arquivo. Atualiza nroNos.
 */
static int aloca_no(CabecalhoArvoreB *cab, FILE *arq)
{
    int rrn;

    if (cab->topo != -1)
    {
        /* Desempilha o no removido do topo: novo topo = proximo do reusado. */
        rrn = cab->topo;
        NoArvoreB reuso;
        noArvore_ler(&reuso, rrn, arq);
        cab->topo = reuso.proximo;
    }
    else
    {
        /* Nenhum no removido disponivel: cresce o arquivo. */
        rrn = cab->proxRRN;
        cab->proxRRN++;
    }

    cab->nroNos++;
    return rrn;
}

/*
 * inserir_ordenado: insere (chave, pr) em vetores temporarios mantendo a
 * ordem crescente das chaves. 'filhoDir' e o ponteiro de subarvore que deve
 * ficar imediatamente a direita da chave inserida (-1 em no folha).
 * Os vetores temporarios comportam ate MAX_CHAVES+1 chaves (overflow).
 */
static void inserir_ordenado(int tC[], int tPR[], int tP[], int *nChaves,
                             int chave, int pr, int filhoDir)
{
    int c = *nChaves;

    /* Encontra a posicao de insercao p (numero de chaves menores que 'chave'). */
    int p = 0;
    while (p < c && tC[p] < chave)
        p++;

    /* Desloca chaves e referencias maiores uma posicao a direita. */
    for (int j = c; j > p; j--)
    {
        tC[j] = tC[j - 1];
        tPR[j] = tPR[j - 1];
    }
    /* Desloca os ponteiros a direita da posicao de insercao. */
    for (int j = c + 1; j > p + 1; j--)
        tP[j] = tP[j - 1];

    tC[p] = chave;
    tPR[p] = pr;
    tP[p + 1] = filhoDir;

    *nChaves = c + 1;
}

/*
 * inserir_rec: insere recursivamente (chave, pr) na subarvore de raiz 'rrn'.
 * Quando a insercao provoca um split que precisa subir, retorna 1 e preenche
 * promoC/promoPR (chave promovida) e filhoDir (RRN do no criado a direita).
 * Caso contrario, retorna 0.
 */
static int inserir_rec(FILE *arq, CabecalhoArvoreB *cab, int rrn,
                       int chave, int pr,
                       int *promoC, int *promoPR, int *filhoDir)
{
    NoArvoreB no;
    noArvore_ler(&no, rrn, arq);

    /* Vetores temporarios com espaco para 1 chave/ponteiro de overflow. */
    int tC[MAX_CHAVES + 1];
    int tPR[MAX_CHAVES + 1];
    int tP[MAX_FILHOS + 1];

    /* Copia o conteudo atual do no para os vetores temporarios. */
    int nChaves = no.nroChaves;
    for (int i = 0; i < nChaves; i++)
    {
        tC[i] = no.C[i];
        tPR[i] = no.PR[i];
    }
    for (int i = 0; i <= nChaves; i++)
        tP[i] = no.P[i];

    if (no.tipoNo == NO_FOLHA)
    {
        /* Caso base: insere diretamente na folha (sem novo filho). */
        inserir_ordenado(tC, tPR, tP, &nChaves, chave, pr, -1);
    }
    else
    {
        /* No interno: escolhe o filho para descer (primeira chave > chave). */
        int i = 0;
        while (i < no.nroChaves && chave > no.C[i])
            i++;

        int novoC, novoPR, novoFilho;
        int split = inserir_rec(arq, cab, no.P[i], chave, pr,
                                &novoC, &novoPR, &novoFilho);

        if (!split)
            return 0; /* subarvore absorveu a insercao; nada a fazer aqui */

        /* O filho dividiu: insere a chave promovida neste no. */
        inserir_ordenado(tC, tPR, tP, &nChaves, novoC, novoPR, novoFilho);
    }

    /* Sem overflow: regrava o no e encerra. */
    if (nChaves <= MAX_CHAVES)
    {
        no.nroChaves = nChaves;
        for (int i = 0; i < MAX_CHAVES; i++)
        {
            no.C[i] = (i < nChaves) ? tC[i] : -1;
            no.PR[i] = (i < nChaves) ? tPR[i] : -1;
        }
        for (int i = 0; i < MAX_FILHOS; i++)
            no.P[i] = (i <= nChaves) ? tP[i] : -1;

        noArvore_escrever(&no, rrn, arq);
        return 0;
    }

    /* ---- Overflow (4 chaves): split em no esquerdo + promovida + direito ----
     * Distribuicao mais uniforme com o no esquerdo recebendo uma chave a mais:
     *   esquerda: chaves 0,1            ponteiros 0,1,2
     *   promovida: chave 2 (1a chave do no resultante a direita)
     *   direita:  chave 3               ponteiros 3,4
     * O no a direita e sempre o no recem-criado. */
    NoArvoreB esq, dir;
    noArvore_inicializar(&esq);
    noArvore_inicializar(&dir);

    /* No da esquerda reutiliza o RRN original. */
    esq.nroChaves = 2;
    esq.C[0] = tC[0]; esq.PR[0] = tPR[0];
    esq.C[1] = tC[1]; esq.PR[1] = tPR[1];
    esq.P[0] = tP[0]; esq.P[1] = tP[1]; esq.P[2] = tP[2];

    /* No da direita (novo). */
    dir.nroChaves = 1;
    dir.C[0] = tC[3]; dir.PR[0] = tPR[3];
    dir.P[0] = tP[3]; dir.P[1] = tP[4];

    /* tipoNo apos o split: folha continua folha; no com filhos vira interno.
     * Um eventual no raiz que se dividiu deixa de ser raiz aqui (vira interno
     * ou folha), pois a nova raiz sera criada no nivel acima. */
    int ehFolha = (tP[0] == -1);
    esq.tipoNo = ehFolha ? NO_FOLHA : NO_INTERMEDIARIO;
    dir.tipoNo = ehFolha ? NO_FOLHA : NO_INTERMEDIARIO;

    /* Grava o no esquerdo no RRN original e o direito em um novo RRN. */
    noArvore_escrever(&esq, rrn, arq);
    int rrnDir = aloca_no(cab, arq);
    noArvore_escrever(&dir, rrnDir, arq);

    /* Propaga a chave promovida para o nivel superior. */
    *promoC = tC[2];
    *promoPR = tPR[2];
    *filhoDir = rrnDir;
    return 1;
}

/* Insercao de alto nivel: trata arvore vazia e criacao de nova raiz. */
void arvore_inserir(FILE *arq, CabecalhoArvoreB *cab, int chave, int pr)
{
    /* Arvore vazia: cria o primeiro no (folha = raiz, tipoNo = -1). */
    if (cab->noRaiz == -1)
    {
        int rrn = aloca_no(cab, arq);

        NoArvoreB raiz;
        noArvore_inicializar(&raiz);
        raiz.tipoNo = NO_FOLHA; /* quando folha = raiz, tipoNo = -1 */
        raiz.nroChaves = 1;
        raiz.C[0] = chave;
        raiz.PR[0] = pr;

        noArvore_escrever(&raiz, rrn, arq);
        cab->noRaiz = rrn;
        return;
    }

    /* Insercao recursiva a partir da raiz. */
    int promoC, promoPR, filhoDir;
    int split = inserir_rec(arq, cab, cab->noRaiz, chave, pr,
                            &promoC, &promoPR, &filhoDir);

    /* Split propagou ate a raiz: cria uma nova raiz com a chave promovida. */
    if (split)
    {
        int rrnRaiz = aloca_no(cab, arq);

        NoArvoreB raiz;
        noArvore_inicializar(&raiz);
        raiz.tipoNo = NO_RAIZ;
        raiz.nroChaves = 1;
        raiz.C[0] = promoC;
        raiz.PR[0] = promoPR;
        raiz.P[0] = cab->noRaiz; /* antigo no raiz vira filho esquerdo */
        raiz.P[1] = filhoDir;    /* no criado no split vira filho direito */

        noArvore_escrever(&raiz, rrnRaiz, arq);
        cab->noRaiz = rrnRaiz;
    }
}

/* ========================================================================
 *  Busca na arvore-B
 * ====================================================================== */

/* Busca 'chave' descendo da raiz ate uma folha; retorna PR ou -1. */
int arvore_buscar(FILE *arq, CabecalhoArvoreB *cab, int chave)
{
    int rrn = cab->noRaiz;

    /* Percorre um caminho da raiz ate uma folha. */
    while (rrn != -1)
    {
        NoArvoreB no;
        noArvore_ler(&no, rrn, arq);

        /* Localiza a primeira chave >= 'chave' no no. */
        int i = 0;
        while (i < no.nroChaves && chave > no.C[i])
            i++;

        /* Chave encontrada neste no. */
        if (i < no.nroChaves && no.C[i] == chave)
            return no.PR[i];

        /* Desce para a subarvore apropriada (-1 em folha encerra a busca). */
        rrn = no.P[i];
    }

    return -1; /* chave inexistente */
}

int arvore_buscar_no(FILE *arq, CabecalhoArvoreB *cab, NoArvoreB *no, int chave)
{
    int rrn = cab->noRaiz;

    /* Percorre um caminho da raiz ate uma folha. */
    while (rrn != -1)
    {
        noArvore_ler(no, rrn, arq);

        /* Localiza a primeira chave >= 'chave' no no. */
        int i = 0;
        while (i < no->nroChaves && chave > no->C[i])
            i++;

        /* Chave encontrada neste no. */
        if (i < no->nroChaves && no->C[i] == chave)
            break;

        /* Desce para a subarvore apropriada (-1 em folha encerra a busca). */
        rrn = no->P[i];
    }

    return rrn; /* chave inexistente */
}
