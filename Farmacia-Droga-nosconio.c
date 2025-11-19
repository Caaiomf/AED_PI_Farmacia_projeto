#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#define TF 100

typedef struct{
    int dia, mes, ano;
} tpdata;

typedef struct{
    int num;
    char cidade[50], estado[3], pais[20], rua[50];
} tpendereco;

typedef struct{
    tpendereco end;
    int cod, ddd, tel;
    char nome[50], cnpj[20];
} tplab;

typedef struct{
    tpdata validade;
    tpdata fab;
    tplab lab;
    int quant, cod, lote;
    float valorC, valorV;
    char desc[50];
} tpproduto;

typedef struct{
    tpendereco end;
    tpdata nasc;
    int ddd, tel;
    char cpf[15], nome[50];
} tpcliente;

typedef struct{
    tpdata datanota;
    tpproduto prod[TF];
    int nf, TLNota;
} tpvendas;

/* Utility: clear screen (uses system call; works on Windows as requested) */
void clear_screen(void){
    system("cls");
}

/* Simple prompt to wait for enter */
void wait_enter(void){
    int c;
    printf("\nPressione ENTER para continuar...");
    while ((c = getchar()) != '\n' && c != EOF) {} /* flush rest of line */
    /* wait for actual enter */
    getchar();
}

/* Read a line into buffer and trim newline */
void read_line(char *buf, int size){
    if(fgets(buf, size, stdin) != NULL){
        size_t ln = strlen(buf) - 1;
        if(buf[ln] == '\n') buf[ln] = '\0';
    } else {
        buf[0] = '\0';
    }
}

/* Read a single option character (first non-space char) */
char read_option_char(void){
    char line[128];
    read_line(line, sizeof(line));
    for (int i = 0; line[i] != '\0'; i++){
        if (!isspace((unsigned char)line[i])) {
            return toupper((unsigned char)line[i]);
        }
    }
    return '\0';
}

/* FILE SEARCH / BUSCA functions return file position or -1 */

int buscacliente(FILE *ptrcli, char cpfaux[15]){
    tpcliente reg;
    if(ptrcli == NULL) return -1;
    fseek(ptrcli, 0, SEEK_SET);
    while(fread(&reg, sizeof(tpcliente), 1, ptrcli) == 1){
        if(strcmp(cpfaux, reg.cpf) == 0){
            return (int)(ftell(ptrcli) - sizeof(tpcliente));
        }
    }
    return -1;
}

int buscalab(FILE *ptrlab, int codaux){
    tplab reg;
    if(ptrlab == NULL) return -1;
    fseek(ptrlab, 0, SEEK_SET);
    while(fread(&reg, sizeof(tplab), 1, ptrlab) == 1){
        if(codaux == reg.cod){
            return (int)(ftell(ptrlab) - sizeof(tplab));
        }
    }
    return -1;
}

int buscaprod(FILE *ptrprod, int codaux){
    tpproduto reg;
    if(ptrprod == NULL) return -1;
    fseek(ptrprod, 0, SEEK_SET);
    while(fread(&reg, sizeof(tpproduto), 1, ptrprod) == 1){
        if(codaux == reg.cod){
            return (int)(ftell(ptrprod) - sizeof(tpproduto));
        }
    }
    return -1;
}

int buscavendas(FILE *ptrvenda, int nfaux){
    tpvendas reg;
    if(ptrvenda == NULL) return -1;
    fseek(ptrvenda, 0, SEEK_SET);
    while(fread(&reg, sizeof(tpvendas), 1, ptrvenda) == 1){
        if(nfaux == reg.nf){
            return (int)(ftell(ptrvenda) - sizeof(tpvendas));
        }
    }
    return -1;
}

/* CPF and CNPJ formatting helpers.
   The original code used in-place transformations; keep same behavior. */

void convertecpf(char cpf[15]){
    /* expects cpf with at least 11 digits (positions 0..10) and will transform it to format XXX.XXX.XXX-XX */
    char tmp[15];
    if(strlen(cpf) < 11) return;
    /* build formatted string */
    snprintf(tmp, sizeof(tmp), "%c%c%c.%c%c%c.%c%c%c-%c%c",
             cpf[0], cpf[1], cpf[2],
             cpf[3], cpf[4], cpf[5],
             cpf[6], cpf[7], cpf[8],
             cpf[9], cpf[10]);
    strcpy(cpf, tmp);
}

void revertecpf(char cpf[15]){
    /* expects formatted cpf; convert back to 11-digit string */
    char tmp[15] = {0};
    int j = 0;
    for (int i = 0; cpf[i] != '\\0' && j < 11; i++){
        if(isdigit((unsigned char)cpf[i])){
            tmp[j++] = cpf[i];
        }
    }
    tmp[j] = '\\0';
    strcpy(cpf, tmp);
}

void convertecnpj(char cnpj[20]){
    /* basic conversion expecting at least 14 digits -> format XX.XXX.XXX/XXXX-XX (approx) */
    char tmp[20];
    if(strlen(cnpj) < 14) return;
    snprintf(tmp, sizeof(tmp), "%c%c.%c%c%c.%c%c%c/%c%c%c%c-%c%c",
             cnpj[0], cnpj[1],
             cnpj[2], cnpj[3], cnpj[4],
             cnpj[5], cnpj[6], cnpj[7],
             cnpj[8], cnpj[9], cnpj[10], cnpj[11],
             cnpj[12], cnpj[13]);
    strcpy(cnpj, tmp);
}

void revertecnpj(char cnpj[20]){
    char tmp[20] = {0};
    int j = 0;
    for (int i = 0; cnpj[i] != '\\0' && j < 14; i++){
        if(isdigit((unsigned char)cnpj[i])){
            tmp[j++] = cnpj[i];
        }
    }
    tmp[j] = '\\0';
    strcpy(cnpj, tmp);
}

/* Validate CPF using the same algorithm as original */
int validcpf(char cpf[15]){
    char raw[15];
    strcpy(raw, cpf);
    revertecpf(raw); /* now raw has only digits, length 11 expected */
    if(strlen(raw) != 11) return 0;
    int soma = 0, i, j, num, dig10, dig11;
    for(i = 0, j = 10; i < 9; i++, j--){
        num = raw[i] - '0';
        soma += num * j;
    }
    int rem = soma % 11;
    if(rem < 2) dig10 = 0; else dig10 = 11 - rem;
    soma = 0;
    for(i = 0, j = 11; i < 9; i++, j--){
        num = raw[i] - '0';
        soma += num * j;
    }
    soma += dig10 * 2;
    rem = soma % 11;
    if(rem < 2) dig11 = 0; else dig11 = 11 - rem;
    if((raw[9] - '0') == dig10 && (raw[10] - '0') == dig11){
        /* restore formatted CPF */
        convertecpf(cpf);
        return 1;
    }
    return 0;
}

/* -- CADASTROS -- */

void cadcliente(void){
    tpcliente cliente;
    FILE *cli = fopen("clientes.dat", "ab+");
    if(cli == NULL){
        printf("Erro ao abrir arquivo de clientes.\n");
        return;
    }
    while(1){
        clear_screen();
        printf("=== CADASTRAR CLIENTE ===\n");
        printf("Digite o CPF (apenas numeros) ou deixe em branco para voltar: ");
        fflush(stdin);
        read_line(cliente.cpf, sizeof(cliente.cpf));
        if(strlen(cliente.cpf) == 0) break;
        convertecpf(cliente.cpf); /* format for storage */
        if(buscacliente(cli, cliente.cpf) != -1){
            printf("Usuario ja cadastrado. Tente outro CPF.\n");
            wait_enter();
            continue;
        }
        if(!validcpf(cliente.cpf)){
            printf("CPF invalido. Digite novamente.\n");
            wait_enter();
            continue;
        }
        printf("Nome: ");
        fflush(stdin);
        read_line(cliente.nome, sizeof(cliente.nome));
        printf("DDD e Telefone (ex: 11 999999999): ");
        if(scanf("%d %d", &cliente.ddd, &cliente.tel) != 2){
            /* flush and continue */
            int c; while((c = getchar()) != '\\n' && c != EOF){}
            printf("Entrada invalida. Voltando...\n");
            wait_enter();
            continue;
        }
        printf("Data de nascimento (dia mes ano): ");
        if(scanf("%d %d %d", &cliente.nasc.dia, &cliente.nasc.mes, &cliente.nasc.ano) != 3){
            int c; while((c = getchar()) != '\\n' && c != EOF){}
            printf("Entrada invalida. Voltando...\n");
            wait_enter();
            continue;
        }
        int c; while((c = getchar()) != '\\n' && c != EOF){} /* flush newline */
        printf("Endereco - Rua: ");
        read_line(cliente.end.rua, sizeof(cliente.end.rua));
        printf("Numero: ");
        if(scanf("%d", &cliente.end.num) != 1){
            int c; while((c = getchar()) != '\\n' && c != EOF){};
            printf("Entrada invalida. Voltando...\n");
            wait_enter();
            continue;
        }
        while((c = getchar()) != '\\n' && c != EOF){};
        printf("Cidade: ");
        read_line(cliente.end.cidade, sizeof(cliente.end.cidade));
        printf("Estado (sigla): ");
        read_line(cliente.end.estado, sizeof(cliente.end.estado));
        printf("Pais: ");
        read_line(cliente.end.pais, sizeof(cliente.end.pais));
        fseek(cli, 0, SEEK_END);
        fwrite(&cliente, sizeof(tpcliente), 1, cli);
        printf("Cliente cadastrado com sucesso!\n");
        wait_enter();
    }
    fclose(cli);
}

void cadlab(void){
    tplab laboratorio;
    FILE *lab = fopen("laboratorios.dat", "ab+");
    if(lab == NULL){
        printf("Erro ao abrir arquivo de laboratorios.\n");
        return;
    }
    while(1){
        clear_screen();
        printf("=== CADASTRAR LABORATORIO ===\n");
        printf("Digite o codigo (1000-9999) ou 0 para voltar: ");
        int codaux = 0;
        if(scanf("%d", &codaux) != 1){
            int c; while((c = getchar()) != '\\n' && c != EOF){};
            printf("Entrada invalida.\n");
            wait_enter();
            continue;
        }
        if(codaux == 0) break;
        if(codaux < 1000 || codaux > 9999){
            printf("Codigo invalido. Deve ter 4 digitos.\n");
            wait_enter();
            continue;
        }
        if(buscalab(lab, codaux) != -1){
            printf("Laboratorio ja cadastrado com esse codigo.\n");
            wait_enter();
            continue;
        }
        laboratorio.cod = codaux;
        int c; while((c = getchar()) != '\\n' && c != EOF){};
        printf("CNPJ (apenas numeros): ");
        read_line(laboratorio.cnpj, sizeof(laboratorio.cnpj));
        convertecnpj(laboratorio.cnpj);
        printf("Razao Social: ");
        read_line(laboratorio.nome, sizeof(laboratorio.nome));
        printf("DDD e Telefone: ");
        if(scanf("%d %d", &laboratorio.ddd, &laboratorio.tel) != 2){
            int c; while((c = getchar()) != '\\n' && c != EOF){};
            printf("Entrada invalida. Voltando...\n");
            wait_enter();
            continue;
        }
        while((c = getchar()) != '\\n' && c != EOF){};
        printf("Endereco - Rua: ");
        read_line(laboratorio.end.rua, sizeof(laboratorio.end.rua));
        printf("Numero: ");
        if(scanf("%d", &laboratorio.end.num) != 1){
            int c; while((c = getchar()) != '\\n' && c != EOF){};
            printf("Entrada invalida. Voltando...\n");
            wait_enter();
            continue;
        }
        while((c = getchar()) != '\\n' && c != EOF){};
        printf("Cidade: ");
        read_line(laboratorio.end.cidade, sizeof(laboratorio.end.cidade));
        printf("Estado: ");
        read_line(laboratorio.end.estado, sizeof(laboratorio.end.estado));
        printf("Pais: ");
        read_line(laboratorio.end.pais, sizeof(laboratorio.end.pais));
        fseek(lab, 0, SEEK_END);
        fwrite(&laboratorio, sizeof(tplab), 1, lab);
        printf("Laboratorio cadastrado com sucesso!\n");
        wait_enter();
    }
    fclose(lab);
}

/* -- CONSULTAS -- */

void consultacliente(void){
    tpcliente cliente;
    FILE *cli = fopen("clientes.dat", "rb");
    if(cli == NULL){
        printf("Erro ao abrir arquivo de clientes.\n");
        return;
    }
    while(1){
        clear_screen();
        printf("=== CONSULTAR CLIENTE ===\n");
        printf("Digite CPF (apenas numeros) ou deixe em branco para voltar: ");
        fflush(stdin);
        read_line(cliente.cpf, sizeof(cliente.cpf));
        if(strlen(cliente.cpf) == 0) break;
        convertecpf(cliente.cpf);
        int pos = buscacliente(cli, cliente.cpf);
        if(pos == -1){
            printf("Usuario nao encontrado.\n");
            wait_enter();
            continue;
        }
        fseek(cli, pos, SEEK_SET);
        fread(&cliente, sizeof(tpcliente), 1, cli);
        printf("\\nCPF: %s\\n", cliente.cpf);
        printf("Nome: %s\\n", cliente.nome);
        printf("Telefone: (%d) %d\\n", cliente.ddd, cliente.tel);
        printf("Nascimento: %d/%d/%d\\n", cliente.nasc.dia, cliente.nasc.mes, cliente.nasc.ano);
        printf("Endereco: %s, %d\\n", cliente.end.rua, cliente.end.num);
        printf("Cidade: %s\\n", cliente.end.cidade);
        printf("Estado: %s\\n", cliente.end.estado);
        printf("Pais: %s\\n", cliente.end.pais);
        wait_enter();
    }
    fclose(cli);
}

void consultalab(void){
    tplab laboratorio;
    FILE *lab = fopen("laboratorios.dat", "rb");
    if(lab == NULL){
        printf("Erro ao abrir arquivo de laboratorios.\n");
        return;
    }
    while(1){
        clear_screen();
        printf("=== CONSULTAR LABORATORIO ===\\n");
        printf("Digite o codigo (1000-9999) ou 0 para voltar: ");
        int codaux = 0;
        if(scanf("%d", &codaux) != 1){
            int c; while((c = getchar()) != '\\n' && c != EOF){};
            printf("Entrada invalida.\\n");
            wait_enter();
            continue;
        }
        if(codaux == 0) break;
        int pos = buscalab(lab, codaux);
        if(pos == -1){
            printf("Laboratorio nao encontrado.\\n");
            wait_enter();
            continue;
        }
        fseek(lab, pos, SEEK_SET);
        fread(&laboratorio, sizeof(tplab), 1, lab);
        printf("\\nCodigo: %d\\n", laboratorio.cod);
        printf("CNPJ: %s\\n", laboratorio.cnpj);
        printf("Razao Social: %s\\n", laboratorio.nome);
        printf("Telefone: (%d) %d\\n", laboratorio.ddd, laboratorio.tel);
        printf("Endereco: %s, %d\\n", laboratorio.end.rua, laboratorio.end.num);
        printf("Cidade: %s\\n", laboratorio.end.cidade);
        printf("Estado: %s\\n", laboratorio.end.estado);
        printf("Pais: %s\\n", laboratorio.end.pais);
        wait_enter();
    }
    fclose(lab);
}

/* -- MENUS SIMPLES -- */

void menuclientes(void){
    char opc = '\0';
    do{
        clear_screen();
        printf("=== CLIENTES ===\\n");
        printf("A - Cadastrar\\n");
        printf("B - Consultar\\n");
        printf("C - Excluir (em construcao)\\n");
        printf("D - Relatorio (em construcao)\\n");
        printf("X - Voltar\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': cadcliente(); break;
            case 'B': consultacliente(); break;
            case 'C': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'D': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

void menulaboratorios(void){
    char opc = '\0';
    do{
        clear_screen();
        printf("=== LABORATORIOS ===\\n");
        printf("A - Cadastrar\\n");
        printf("B - Consultar\\n");
        printf("C - Excluir (em construcao)\\n");
        printf("D - Relatorio (em construcao)\\n");
        printf("X - Voltar\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': cadlab(); break;
            case 'B': consultalab(); break;
            case 'C': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'D': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

void menuprodutos(void){
    char opc = '\0';
    do{
        clear_screen();
        printf("=== PRODUTOS ===\\n");
        printf("A - Cadastrar (em construcao)\\n");
        printf("B - Consultar (em construcao)\\n");
        printf("C - Excluir (em construcao)\\n");
        printf("D - Relatorio (em construcao)\\n");
        printf("E - Promocao (em construcao)\\n");
        printf("X - Voltar\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'B': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'C': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'D': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'E': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

void menuvendas(void){
    char opc = '\0';
    do{
        clear_screen();
        printf("=== VENDAS ===\\n");
        printf("A - Cadastrar (em construcao)\\n");
        printf("B - Consultar (em construcao)\\n");
        printf("C - Excluir (em construcao)\\n");
        printf("D - Relatorio (em construcao)\\n");
        printf("X - Voltar\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'B': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'C': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'D': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

void menurelatorios(void){
    char opc = '\\0';
    do{
        clear_screen();
        printf("=== RELATORIOS ===\\n");
        printf("A - Prod. para vencer (em construcao)\\n");
        printf("B - Baixo estoque (em construcao)\\n");
        printf("C - Vendas no mes (em construcao)\\n");
        printf("D - Media de compra (em construcao)\\n");
        printf("X - Voltar\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'B': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'C': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'D': printf(\"Em Construcao...\\n\"); wait_enter(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

void menu(void){
    char opc = '\\0';
    do{
        clear_screen();
        printf("=== MENU PRINCIPAL ===\\n");
        printf("A - Clientes\\n");
        printf("B - Laboratorios\\n");
        printf("C - Produtos\\n");
        printf("D - Vendas\\n");
        printf("E - Relatorios\\n");
        printf("X - Sair\\n");
        printf("Opcao: ");
        opc = read_option_char();
        switch(opc){
            case 'A': menuclientes(); break;
            case 'B': menulaboratorios(); break;
            case 'C': menuprodutos(); break;
            case 'D': menuvendas(); break;
            case 'E': menurelatorios(); break;
            case 'X': break;
            default: printf(\"Opcao invalida.\\n\"); wait_enter(); break;
        }
    } while(opc != 'X');
}

int main(void){
    setlocale(LC_ALL, \"Portuguese\");
    menu();
    clear_screen();
    printf(\"Encerrando...\\n\");
    return 0;
}
