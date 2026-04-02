#include "crud.h"

int main(int argc, char *argv[]) {
    int escolha;
    scanf("%d", &escolha);
    if (escolha == 1) {
        lerCsv();
    }
    return 0;
}