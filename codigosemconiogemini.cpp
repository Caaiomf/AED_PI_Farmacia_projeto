#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

// CONSTANTES
#define TF 100

// STRUCTS

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

// FUNÇÕES UTILITÁRIAS

void limparTela() {
    system("cls");
}

void pausarTela() {
    printf("\n");
    system("pause");
}

// FUNÇÕES DO CPF E CNPJ

void convertecpf(char cpf[15]){
    char aux[15];
    strcpy(aux, cpf);
    cpf[14]='\0';
    cpf[13]=cpf[10];
    cpf[12]=cpf[9];
    cpf[11]='-';
    cpf[10]=cpf[8];
    cpf[9]=cpf[7];
    cpf[8]=cpf[6];
    cpf[7]='.';
    cpf[6]=cpf[5];
    cpf[5]=cpf[4];
    cpf[4]=cpf[3];
    cpf[3]='.';
}

void revertecpf(char cpf[15]){
    cpf[3]=cpf[4];
    cpf[4]=cpf[5];
    cpf[5]=cpf[6];
    cpf[6]=cpf[8];
    cpf[7]=cpf[9];
    cpf[8]=cpf[10];
    cpf[9]=cpf[12];
    cpf[10]=cpf[13];
    cpf[11]='\0';
}

void convertecnpj(char cnpj[20]){
    cnpj[18]='\0';
    cnpj[17]=cnpj[13];
    cnpj[16]=cnpj[12];
    cnpj[15]='-';
    cnpj[14]=cnpj[11];
    cnpj[13]=cnpj[10];
    cnpj[12]=cnpj[9];
    cnpj[11]=cnpj[8];
    cnpj[10]='/';
    cnpj[9]=cnpj[7];
    cnpj[8]=cnpj[6];
    cnpj[7]=cnpj[5];
    cnpj[6]='.';
    cnpj[5]=cnpj[4];
    cnpj[4]=cnpj[3];
    cnpj[3]=cnpj[2];
    cnpj[2]='.';
}

void revertecnpj(char cnpj[20]){
    cnpj[2]=cnpj[3];
    cnpj[3]=cnpj[4];
    cnpj[4]=cnpj[5];
    cnpj[5]=cnpj[7];
    cnpj[6]=cnpj[8];
    cnpj[7]=cnpj[9];
    cnpj[8]=cnpj[11];
    cnpj[9]=cnpj[12];
    cnpj[10]=cnpj[13];
    cnpj[11]=cnpj[14];
    cnpj[12]=cnpj[16];
    cnpj[13]=cnpj[17];
    cnpj[14]='\0';
}

int validcpf(char cpf[15]){
    int soma=0, i, j=10, num;
    char resto[3], codv[3], n;
    
    revertecpf(cpf);
    
    codv[0]=cpf[9];
    codv[1]=cpf[10];
    codv[2]='\0';
    resto[2]='\0';
    
    for(i=0;i<9;i++, j--){
        n=cpf[i];
        num=n-'0';
        soma+=num*j;
    }
    num=soma%11;
    if(num<2) num=0;
    else num=11-num;
    
    resto[0]='0'+num;
    
    soma=0;
    j=11;
    for(i=0;i<9;i++, j--){
        n=cpf[i];
        num=n-'0';
        soma+=num*j;
    }
    
    n=resto[0];
    num=n-'0';
    soma+=num*2;
    num=soma%11;
    if(num<2) num=0;
    else num=11-num;
    
    resto[1]='0'+num;
    
    convertecpf(cpf);
    
    if(strcmp(resto, codv)==0)
        return 1;
    else
        return 0;
}

// BUSCAS EM ARQUIVO

int buscacliente(FILE *ptrcli, char cpfaux[15]){
    tpcliente reg;
    fseek(ptrcli,0,0);
    while(fread(&reg, sizeof(tpcliente), 1, ptrcli) == 1){
        if(strcmp(cpfaux, reg.cpf)==0){
            fseek(ptrcli, -((long)sizeof(tpcliente)), SEEK_CUR);
            return ftell(ptrcli);
        }
    }
    return -1;
}

int buscalab(FILE *ptrlab, int codaux){
    tplab reg;
    fseek(ptrlab,0,0);
    while(fread(&reg, sizeof(tplab), 1, ptrlab) == 1){
        if(codaux == reg.cod){
            fseek(ptrlab, -((long)sizeof(tplab)), SEEK_CUR);
            return ftell(ptrlab);
        }
    }
    return -1;
}

// CADASTROS

void cadcliente(){
    tpcliente cliente;
    FILE *cli;
    cli = fopen("clientes.dat","ab+");
    
    if(cli == NULL){
        printf("\nErro na abertura do arquivo!\n");
        pausarTela();
        return;
    }

    while(1) {
        limparTela();
        printf("--- CADASTRO DE CLIENTE ---\n");
        printf("Digite o CPF (somente numeros) ou ENTER para voltar: ");
        
        fflush(stdin); 
        gets(cliente.cpf);

        if(strcmp(cliente.cpf, "") == 0) break; 

        convertecpf(cliente.cpf);

        if(buscacliente(cli, cliente.cpf) != -1){
            printf("\nERRO: Usuario ja cadastrado.\n");
            pausarTela();
            continue;
        }

        if(validcpf(cliente.cpf) != 1){
            printf("\nERRO: CPF invalido.\n");
            pausarTela();
            continue;
        }

        printf("Nome: ");
        fflush(stdin);
        gets(cliente.nome);

        printf("DDD: ");
        scanf("%d", &cliente.ddd);
        printf("Telefone: ");
        scanf("%d", &cliente.tel);

        printf("Data Nascimento (Dia Mes Ano): ");
        scanf("%d %d %d", &cliente.nasc.dia, &cliente.nasc.mes, &cliente.nasc.ano);
        
        printf("Rua: ");
        fflush(stdin); 
        gets(cliente.end.rua);

        printf("Numero da casa: ");
        scanf("%d", &cliente.end.num);

        printf("Cidade: ");
        fflush(stdin);
        gets(cliente.end.cidade);

        printf("Estado (sigla): ");
        fflush(stdin);
        gets(cliente.end.estado);

        printf("Pais: ");
        fflush(stdin);
        gets(cliente.end.pais);

        fseek(cli, 0, SEEK_END);
        fwrite(&cliente, sizeof(tpcliente), 1, cli);
        printf("\n>>> Cliente cadastrado com sucesso! <<<\n");
        pausarTela();
    }
    fclose(cli);
}

void cadlab(){
    int codaux;
    tplab laboratorio;
    FILE *lab;
    lab = fopen("laboratorios.dat","ab+");
    
    if(lab == NULL){
        printf("\nErro na abertura do arquivo!\n");
        pausarTela();
        return;
    }

    while(1){
        limparTela();
        printf("--- CADASTRO DE LABORATORIO ---\n");
        printf("Codigo (4 digitos) ou 0 para voltar: ");
        scanf("%d", &codaux);
        fflush(stdin); 

        if(codaux == 0) break;

        if(codaux < 1000 || codaux > 9999){
            printf("\nO codigo deve ter 4 digitos.\n");
            pausarTela();
            continue;
        }

        if(buscalab(lab, codaux) != -1){
            printf("\nLaboratorio ja cadastrado.\n");
            pausarTela();
            continue;
        }

        laboratorio.cod = codaux;
        
        printf("CNPJ: ");
        fflush(stdin);
        gets(laboratorio.cnpj);
        convertecnpj(laboratorio.cnpj);

        printf("Razao Social: ");
        fflush(stdin);
        gets(laboratorio.nome);

        printf("DDD: ");
        scanf("%d", &laboratorio.ddd);
        printf("Telefone: ");
        scanf("%d", &laboratorio.tel);

        printf("Rua: ");
        fflush(stdin);
        gets(laboratorio.end.rua);

        printf("Numero: ");
        scanf("%d", &laboratorio.end.num);

        printf("Cidade: ");
        fflush(stdin);
        gets(laboratorio.end.cidade);

        printf("Estado: ");
        fflush(stdin);
        gets(laboratorio.end.estado);

        printf("Pais: ");
        fflush(stdin);
        gets(laboratorio.end.pais);

        fseek(lab, 0, SEEK_END);
        fwrite(&laboratorio, sizeof(tplab), 1, lab);
        printf("\n>>> Laboratorio Salvo! <<<\n");
        pausarTela();
    }
    fclose(lab);
}

// CONSULTAS

void consultacliente(){
    tpcliente cliente;
    FILE *cli;
    char cpfBusca[15];
    
    cli = fopen("clientes.dat","rb+");
    if(cli == NULL){
        printf("\nArquivo nao encontrado.\n");
        pausarTela();
        return;
    }

    while(1){
        limparTela();
        printf("--- CONSULTA DE CLIENTE ---\n");
        printf("Digite o CPF (ou ENTER para voltar): ");
        
        fflush(stdin);
        gets(cpfBusca);

        if(strcmp(cpfBusca, "") == 0) break;

        convertecpf(cpfBusca);

        if(buscacliente(cli, cpfBusca) == -1){
            printf("\nCliente nao encontrado.\n");
            pausarTela();
        } else {
            fread(&cliente, sizeof(tpcliente), 1, cli);
            
            printf("\n--- DADOS DO CLIENTE ---\n");
            printf("CPF: %s\n", cliente.cpf);
            printf("Nome: %s\n", cliente.nome);
            printf("Tel: (%d) %d\n", cliente.ddd, cliente.tel);
            printf("Nasc: %d/%d/%d\n", cliente.nasc.dia, cliente.nasc.mes, cliente.nasc.ano);
            printf("Endereco: %s, %d\n", cliente.end.rua, cliente.end.num);
            printf("Local: %s - %s, %s\n", cliente.end.cidade, cliente.end.estado, cliente.end.pais);
            
            pausarTela();
        }
    }
    fclose(cli);
}

void consultalab(){
    int codaux;
    tplab laboratorio;
    FILE *lab;
    lab = fopen("laboratorios.dat","rb+");
    
    if(lab == NULL){
        printf("\nArquivo nao encontrado.\n");
        pausarTela();
        return;
    }

    while(1){
        limparTela();
        printf("--- CONSULTA LABORATORIO ---\n");
        printf("Codigo (0 para voltar): ");
        scanf("%d", &codaux);
        fflush(stdin);

        if(codaux == 0) break;

        if(buscalab(lab, codaux) == -1){
            printf("\nNao encontrado.\n");
            pausarTela();
        } else {
            fread(&laboratorio, sizeof(tplab), 1, lab);
            printf("\nCodigo: %d\n", laboratorio.cod);
            printf("CNPJ: %s\n", laboratorio.cnpj);
            printf("Nome: %s\n", laboratorio.nome);
            printf("Tel: (%d) %d\n", laboratorio.ddd, laboratorio.tel);
            printf("End: %s, %d - %s\n", laboratorio.end.rua, laboratorio.end.num, laboratorio.end.cidade);
            pausarTela();
        }
    }
    fclose(lab);
}

// MENUS

void menuclientes(){
    char opc;
    do{
        limparTela();
        printf("=== MENU CLIENTES ===\n");
        printf("[A] - Cadastrar\n");
        printf("[B] - Consultar\n");
        printf("[C] - Excluir (Em construcao)\n");
        printf("[D] - Relatorio (Em construcao)\n");
        printf("[V] - Voltar\n");
        printf("Opcao: ");
        
        scanf(" %c", &opc);
        opc = toupper(opc); 

        switch(opc){
            case 'A': cadcliente(); break;
            case 'B': consultacliente(); break;
            case 'C': 
            case 'D':
                printf("\nEm Construcao...\n");
                pausarTela();
                break;
        }
    } while(opc != 'V');
}

void menulaboratorios(){
    char opc;
    do{
        limparTela();
        printf("=== MENU LABORATORIOS ===\n");
        printf("[A] - Cadastrar\n");
        printf("[B] - Consultar\n");
        printf("[C] - Excluir\n");
        printf("[D] - Relatorio\n");
        printf("[V] - Voltar\n");
        printf("Opcao: ");
        
        scanf(" %c", &opc);
        opc = toupper(opc);

        switch(opc){
            case 'A': cadlab(); break;
            case 'B': consultalab(); break;
            case 'C': 
            case 'D':
                printf("\nEm Construcao...\n");
                pausarTela();
                break;
        }
    } while(opc != 'V');
}

void menuprodutos(){
    char opc;
    do{
        limparTela();
        printf("=== MENU PRODUTOS ===\n");
        printf("Em construcao...\n");
        printf("[V] - Voltar\n");
        
        scanf(" %c", &opc);
        opc = toupper(opc);
    } while(opc != 'V');
}

void menuvendas(){
    char opc;
    do{
        limparTela();
        printf("=== MENU VENDAS ===\n");
        printf("Em construcao...\n");
        printf("[V] - Voltar\n");
        
        scanf(" %c", &opc);
        opc = toupper(opc);
    } while(opc != 'V');
}

void menurelatorios(){
    char opc;
    do{
        limparTela();
        printf("=== MENU RELATORIOS ===\n");
        printf("Em construcao...\n");
        printf("[V] - Voltar\n");
        
        scanf(" %c", &opc);
        opc = toupper(opc);
    } while(opc != 'V');
}

// MAIN

int main(void){
    char opc;
    
    do{
        limparTela();
        printf("==========================\n");
        printf("      DROGA+ FARMACIA     \n");
        printf("==========================\n");
        printf("[A] - Clientes\n");
        printf("[B] - Laboratorios\n");
        printf("[C] - Produtos\n");
        printf("[D] - Vendas\n");
        printf("[E] - Relatorios\n");
        printf("[S] - SAIR\n");
        printf("--------------------------\n");
        printf("Escolha uma opcao: ");
        
        scanf(" %c", &opc); 
        opc = toupper(opc);

        switch(opc){
            case 'A': menuclientes(); break;
            case 'B': menulaboratorios(); break;
            case 'C': menuprodutos(); break;
            case 'D': menuvendas(); break;
            case 'E': menurelatorios(); break;
            case 'S':
                printf("\nSaindo...\n");
                break;
            default:
                printf("\nOpcao invalida!\n");
                pausarTela();
        }
    } while(opc != 'S');

    return 0;
}
