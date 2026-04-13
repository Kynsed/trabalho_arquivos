#include "crud.h"

int main(void) 
{
    int escolha, qnt;
    char entradaBin[100];
    
    scanf("%d", &escolha);
    
    switch (escolha) 
    {
        case 1:
            ler();
            break;
        
        case 2:
            scanf("%s", entradaBin);
            selecionar(entradaBin);
            break;
        
        case 3:
            scanf(" %s %d", entradaBin, &qnt);
            buscar(entradaBin, qnt);
            break;
        
        case 4:
            scanf(" %s", entradaBin);
            deletar(entradaBin);
            break;
        
        case 5:
            scanf(" %s %d", entradaBin, &qnt);
            inserir(entradaBin, qnt);
            break;
        
        case 6:
            scanf(" %s", entradaBin);
            atualizar(entradaBin);
            break;

        default:
            printf("Escolha inválida.\n");
            break;
    }
    return 0;
}
