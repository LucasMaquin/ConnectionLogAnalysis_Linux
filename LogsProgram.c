#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINE_LENGTH 100

struct LogLine {
    char line[LINE_LENGTH];
    struct LogLine *anterior;
    struct LogLine *proximo;
};
typedef struct LogLine LogLine;

struct descritor_lista {
    LogLine *inicio;
    LogLine *fim;
    int tamanho;
};
typedef struct descritor_lista TipoDescritorLista;

// --------- Prototipo de funcoes finalizadas -----------------
void criar_lista(TipoDescritorLista *descritor_lista);
int verificar_lista_vazia (TipoDescritorLista* descritor_lista);
void imprimir_toda_lista (TipoDescritorLista* descritor_lista);
void liberar_lista (TipoDescritorLista* descritor_lista);
void inserir_lista_inicio (TipoDescritorLista* descritor_lista, char line[LINE_LENGTH]);
int pesquisar_em_arquivo_log(FILE *pont_arqToRead, TipoDescritorLista* descritor_lista);
int inserindoEmArquivo(FILE *pont_arqToWrite, TipoDescritorLista* descritor_lista);
void VerificarAtualizacoesLogs();
void deletarLog(TipoDescritorLista* descritor_lista, char *horaLog);
// ------------------------------------------------------------

//-------------------------FUNCAO PRINCIPAL ----------------------------
int main () {
    system("clear");
    //lista duplamente encadeadas
    TipoDescritorLista *descritor_lista;
    descritor_lista = (TipoDescritorLista*) malloc(sizeof(TipoDescritorLista));
    criar_lista(descritor_lista);

    char *fileToRead = "/var/log/syslog";
    FILE *pont_arqToRead = fopen(fileToRead, "r");
    pesquisar_em_arquivo_log(pont_arqToRead, descritor_lista);

    char impressaoCheck[3];
    printf("Deseja que seja impresso todos os logs existentes ate o momento verificando o status de conexao? (sim/nao)");
    scanf("%s", impressaoCheck);

    if(strcasecmp(impressaoCheck, "SIM") == 0)
        imprimir_toda_lista(descritor_lista); getchar();

    char CheckDeletar[3];
    char horaLog[9];
    printf("Deseja deletar algum dos logs?(sim/nao)");
    scanf("%s", CheckDeletar);
    if(strcasecmp(CheckDeletar, "SIM") == 0){
        printf("Digite a hora:minuto:segundo que o log foi impresso. (hh:mm:ss)");
        scanf("%s", horaLog);
        deletarLog(descritor_lista, horaLog); getchar();
        printf("\nImprimindo Lista atualizada!\n");
        imprimir_toda_lista(descritor_lista); getchar();
    }

    char arquivoCheck[3];
    char nomeDoArquivo[20];

    FILE *pont_arqToWrite;
    printf("Gostaria de criar um arquivo no mesmo diretorio onde voce rodou o programa? nele ficara armazenados todos os logs atualizados (sim/nao)");
    scanf("%s", arquivoCheck);

    if(strcasecmp(arquivoCheck, "SIM") == 0) {
        printf("Qual o nome do arquivo que voce deseja?");
        scanf("%s", nomeDoArquivo);
        pont_arqToWrite = fopen(strcat(nomeDoArquivo, ".txt"), "a");
        inserindoEmArquivo(pont_arqToWrite, descritor_lista);
        sleep(3);
        printf("Dados armazenados em arquivo\n");
    }


    char CheckVerificacoes[3];
    printf("Deseja criar um processo para verificar novos eventos de conexao e desconexao de rede? (sim/nao)");
    scanf("%s", CheckVerificacoes);

    if(strcasecmp(CheckVerificacoes, "SIM") == 0)
        VerificarAtualizacoesLogs(); getchar();

    //Liberando Lista e fechando arquivos
    liberar_lista(descritor_lista);
    fclose(pont_arqToRead);
    fclose(pont_arqToWrite);

    return 0;
}

//----------------------------------------------------------------------

/// Criando uma lista duplamente encadeada vazia
void criar_lista(TipoDescritorLista *descritor_lista) {
    descritor_lista->inicio = NULL;
    descritor_lista->fim = NULL;
    descritor_lista->tamanho = 0;
}

/// Verificando se lista esta vazia
int verificar_lista_vazia (TipoDescritorLista* descritor_lista) {
    return (descritor_lista->inicio == NULL);
}

/// Imprimindo nodos do inicio ao fim
void imprimir_toda_lista (TipoDescritorLista* descritor_lista) {
    if (verificar_lista_vazia(descritor_lista))
        printf("Lista vazia\n");
    else {
        LogLine* paux;
        for (paux = descritor_lista->inicio; paux != NULL; paux = paux->proximo)
            printf("%s", paux->line);
    }
}

/// Limpando Lista Duplamente Encadeada
void liberar_lista (TipoDescritorLista* descritor_lista) {
    LogLine *paux = descritor_lista->inicio;
    LogLine *aux;
    while (paux != NULL) {
        aux = paux->proximo;
        free(paux);
        paux = aux;
    }
    criar_lista(descritor_lista);
}

/// Inserindo Nodos no fim da lista para ficarem com uma ordem cronologica na impressao
void inserir_lista_inicio (TipoDescritorLista* descritor_lista, char line[LINE_LENGTH]) {
    LogLine* novo = (LogLine*) malloc(sizeof(LogLine));
    strcpy(novo->line, line);

    if (!descritor_lista->inicio) {
        novo->proximo  = NULL;
        novo->anterior = NULL;
        descritor_lista->inicio = novo;
    }
    else {
        novo->proximo = NULL;
        novo->anterior = descritor_lista->fim;
        descritor_lista->fim->proximo = novo;
    }
    descritor_lista->fim = novo;
    descritor_lista->tamanho++;
}

/// Pesquisa em arquivo syslog as linhas compativeis com online e offline
int pesquisar_em_arquivo_log(FILE *pont_arqToRead, TipoDescritorLista* descritor_lista){
    char stringsToFind[2][50] = {"online", "offline"};
    char line[LINE_LENGTH];

    if(pont_arqToRead == NULL){
        printf("Failed to open file");
        return 1;
    }

    while(fgets(line, sizeof(line), pont_arqToRead)){
        for(int i = 0; i < 2; i++){
           if(strstr(line, stringsToFind[i]))
                inserir_lista_inicio(descritor_lista, line);
        }
    }

}

/// Percorre todos os nodos e insere eles dentro de um arquivo
int inserindoEmArquivo(FILE *pont_arqToWrite, TipoDescritorLista* descritor_lista){
    if(pont_arqToWrite == NULL){
        printf("Failed to open file");
        return 1;
    }
    LogLine* paux = descritor_lista->inicio;
    while (paux != NULL) {
        fprintf(pont_arqToWrite, "%s", paux->line);
        paux = paux->proximo;
    }
}


/// Funcao para abrir um processo para verificar novos eventos de online ou offline
void VerificarAtualizacoesLogs(){
    system("tail -f /var/log/syslog |egrep -w 'online|offline'");
}

/// Funcao para deletar log dependendo da hora selecionada
void deletarLog(TipoDescritorLista* descritor_lista, char *horaLog) {
    LogLine* anterior_paux = NULL;
    LogLine* paux = descritor_lista->inicio;

    while (paux != NULL && strstr(paux->line, horaLog) == 0) {
        anterior_paux = paux;
        paux = paux->proximo;
    }
    if (paux == NULL)
        return;
    if (anterior_paux == NULL) {
        descritor_lista->inicio = paux->proximo;
        descritor_lista->inicio->anterior = NULL;
    }
    else {
        if (paux->proximo == NULL) {
            anterior_paux->proximo = paux->proximo;
            descritor_lista->fim = anterior_paux;
        }
        else {
            anterior_paux->proximo = paux->proximo;
            paux->proximo->anterior = anterior_paux;
        }
    }
    free(paux);
}
