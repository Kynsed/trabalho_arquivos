#include "crud.h"

int main(int argc, char *argv[]) {
    int escolha, qnt;
    char entradaBin[100];
    scanf("%d", &escolha);
    switch (escolha) {
        case 1:
            lerCsv();
            break;
        
        case 2:
            scanf("%s", entradaBin);
            select_from(entradaBin);
            break;
        case 3:
            // implementar a função de leitura do arquivo binário e exibição dos dados
            scanf(" %s %d", entradaBin, &qnt);
            busca(entradaBin, qnt);
            break;
        default:
            printf("Escolha inválida.\n");
            break;
    }
    return 0;
}