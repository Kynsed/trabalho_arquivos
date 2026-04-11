#include "crud.h"

int main(int argc, char *argv[]) 
{
    int escolha, qnt;
    char entradaBin[100];
    
    scanf("%d", &escolha);
    
    switch (escolha) 
    {
        case 1:
            lerCsv();
            break;
        
        case 2:
            scanf("%s", entradaBin);
            select_from(entradaBin);
            break;
        
        case 3:
            scanf(" %s %d", entradaBin, &qnt);
            busca(entradaBin, qnt);
            break;
        
        case 4:
            scanf(" %s", entradaBin);
            delete_from(entradaBin);
            break;
        
        case 5:
            scanf(" %s %d", entradaBin, &qnt);
            inserir(entradaBin, qnt);
            break;
        
        case 6:
            scanf(" %s", entradaBin);
            update(entradaBin);
            break;

        default:
            printf("Escolha inválida.\n");
            break;
    }
    return 0;
}