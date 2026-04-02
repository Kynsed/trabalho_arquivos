#include "crud.h"

int main(int argc, char *argv[]) {
    int escolha;
    scanf("%d", &escolha);
    switch (escolha) {
        case 1:
            lerCsv();
            break;
        case 3:
            // implementar a função de leitura do arquivo binário e exibição dos dados
            busca();
            break;
        default:
            printf("Escolha inválida.\n");
            break;
    }
    return 0;
}