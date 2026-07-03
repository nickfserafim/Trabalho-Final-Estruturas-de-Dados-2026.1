#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configurações de Cores ANSI ---
#define RESET "\033[0m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"

#define MAX_PAGES 100 // Limite artificial para testar "Pilha Cheia"

// --- Estruturas de Dados ---
typedef struct Page {
    int id;
    char url[100];
} Page;

typedef struct Node {
    Page data;
    struct Node* next;
} Node;

typedef struct Stack {
    Node* top;
    int count;          // Elementos atuais na pilha
    int total_visited;  // Total histórico de páginas visitadas
} Stack;

// --- Funções de Manipulação da Pilha ---

void initStack(Stack* s) {
    s->top = NULL;
    s->count = 0;
    s->total_visited = 0;
}

int isEmpty(Stack* s) {
    return s->top == NULL;
}

int isFull(Stack* s) {
    return s->count >= MAX_PAGES;
}

int push(Stack* s, Page p) {
    if (isFull(s)) {
        printf(RED "Erro: A pilha está cheia! (Limite de %d atingido)\n" RESET, MAX_PAGES);
        return 0;
    }

    // Verifica duplicidade do ID (para evitar IDs iguais na pilha)
    Node* current = s->top;
    while (current != NULL) {
        if (current->data.id == p.id) {
            printf(RED "Erro: Uma página com o ID %d já existe no histórico!\n" RESET, p.id);
            return 0;
        }
        current = current->next;
    }

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf(RED "Erro: Falha na alocação de memória.\n" RESET);
        return 0;
    }

    newNode->data = p;
    newNode->next = s->top;
    s->top = newNode;
    s->count++;
    s->total_visited++;
    return 1;
}

int pop(Stack* s, Page* p) {
    if (isEmpty(s)) {
        printf(YELLOW "Aviso: A pilha está vazia. Não há páginas para voltar.\n" RESET);
        return 0;
    }

    Node* temp = s->top;
    *p = temp->data;
    s->top = s->top->next;
    free(temp);
    s->count--;
    return 1;
}

int peek(Stack* s, Page* p) {
    if (isEmpty(s)) {
        printf(YELLOW "Aviso: A pilha está vazia.\n" RESET);
        return 0;
    }
    *p = s->top->data;
    return 1;
}

void clearStack(Stack* s) {
    Page p;
    while (!isEmpty(s)) {
        pop(s, &p);
    }
    printf(GREEN "Histórico limpo com sucesso!\n" RESET);
}

// --- Funções de Arquivo (CSV e TXT) ---

void saveCSV(Stack* s, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf(RED "Erro ao abrir o arquivo %s para salvar.\n" RESET, filename);
        return;
    }

    // Salva na primeira linha o total de visitadas para não perder o relatório
    fprintf(file, "TOTAL_VISITED,%d\n", s->total_visited);

    // Salva a pilha do topo para a base
    Node* current = s->top;
    while (current != NULL) {
        fprintf(file, "%d,%s\n", current->data.id, current->data.url);
        current = current->next;
    }

    fclose(file);
    printf(GREEN "Dados salvos em %s com sucesso!\n" RESET, filename);
}

void loadCSV(Stack* s, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf(YELLOW "Arquivo %s não encontrado. Iniciando pilha vazia.\n" RESET, filename);
        return;
    }

    clearStack(s); // Limpa pilha atual antes de carregar
    s->total_visited = 0; // Reseta contador geral

    char line[150];
    Page tempArray[MAX_PAGES];
    int count = 0;

    // Lendo a primeira linha (Total visitado)
    if (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "TOTAL_VISITED", 13) == 0) {
            sscanf(line, "TOTAL_VISITED,%d", &s->total_visited);
        }
    }

    // Lendo as páginas
    while (fgets(line, sizeof(line), file) && count < MAX_PAGES) {
        sscanf(line, "%d,%99[^\n]", &tempArray[count].id, tempArray[count].url);
        count++;
    }
    fclose(file);

    // Para preservar a ordem da pilha, precisamos inserir de trás para frente (base primeiro)
    for (int i = count - 1; i >= 0; i--) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = tempArray[i];
        newNode->next = s->top;
        s->top = newNode;
        s->count++;
    }

    printf(GREEN "Dados carregados de %s com sucesso! Ordem preservada.\n" RESET, filename);
}

void exportTXT(Stack* s, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf(RED "Erro ao criar relatório TXT.\n" RESET);
        return;
    }

    fprintf(file, "========================================\n");
    fprintf(file, "      RELATORIO DE NAVEGACAO\n");
    fprintf(file, "========================================\n");
    fprintf(file, "Total de paginas visitadas (Historico): %d\n", s->total_visited);
    fprintf(file, "Paginas atualmente na pilha: %d\n\n", s->count);
    
    fprintf(file, "--- Estado Atual da Pilha (Topo para Base) ---\n");
    Node* current = s->top;
    if (current == NULL) {
        fprintf(file, "Pilha vazia.\n");
    } else {
        while (current != NULL) {
            fprintf(file, "ID: %d | URL: %s\n", current->data.id, current->data.url);
            current = current->next;
        }
    }
    fprintf(file, "========================================\n");
    fclose(file);
    printf(GREEN "Relatório exportado para %s com sucesso!\n" RESET, filename);
}

// --- Interface do Usuário ---

void showMenu() {
    printf("\n" CYAN "========================================" RESET "\n");
    printf(BLUE "       NAVEGADOR - HISTÓRICO" RESET "\n");
    printf(CYAN "========================================" RESET "\n");
    printf("1. Visitar nova página (Push)\n");
    printf("2. Voltar página (Pop)\n");
    printf("3. Ver página atual (Peek)\n");
    printf("4. Limpar histórico (Clear)\n");
    printf("5. Salvar dados (CSV)\n");
    printf("6. Carregar dados (CSV)\n");
    printf("7. Exportar Relatório (TXT)\n");
    printf("8. Exibir Status da Pilha\n");
    printf("0. Sair\n");
    printf(CYAN "========================================" RESET "\n");
    printf("Escolha uma opcao: ");
}

int main() {
    Stack history;
    initStack(&history);
    
    int option;
    Page p;
    char confirm;

    // Tenta carregar dados iniciais (opcional, pode ser comentado)
    loadCSV(&history, "dados.csv");

    do {
        showMenu();
        if (scanf("%d", &option) != 1) {
            // Tratamento de entrada inválida (se o usuário digitar letra no lugar de número)
            while (getchar() != '\n'); 
            printf(RED "Entrada invalida! Digite um numero.\n" RESET);
            continue;
        }

        switch (option) {
            case 1:
                printf("Digite o ID da pagina: ");
                scanf("%d", &p.id);
                while (getchar() != '\n'); // limpa buffer
                printf("Digite a URL (ex: www.site.com): ");
                scanf("%99[^\n]", p.url);
                if (push(&history, p)) {
                    printf(GREEN "Página '%s' visitada!\n" RESET, p.url);
                }
                break;
            case 2:
                if (pop(&history, &p)) {
                    printf(GREEN "Voce voltou da pagina '%s'.\n" RESET, p.url);
                }
                break;
            case 3:
                if (peek(&history, &p)) {
                    printf(BLUE "Pagina Atual -> ID: %d | URL: %s\n" RESET, p.id, p.url);
                }
                break;
            case 4:
                printf(YELLOW "Tem certeza que deseja limpar TODA a pilha? (s/n): " RESET);
                while (getchar() != '\n');
                scanf("%c", &confirm);
                if (confirm == 's' || confirm == 'S') {
                    clearStack(&history);
                } else {
                    printf("Operacao cancelada.\n");
                }
                break;
            case 5:
                saveCSV(&history, "dados.csv");
                break;
            case 6:
                loadCSV(&history, "dados.csv");
                break;
            case 7:
                exportTXT(&history, "relatorio.txt");
                break;
            case 8:
                printf(BLUE "\n--- STATUS DA PILHA ---\n" RESET);
                printf("Paginas na pilha agora: %d\n", history.count);
                printf("Total historico de visitas: %d\n", history.total_visited);
                break;
            case 0:
                printf(GREEN "Saindo e salvando dados automaticamente...\n" RESET);
                saveCSV(&history, "dados.csv");
                clearStack(&history); // Libera memória antes de sair
                break;
            default:
                printf(RED "Opcao invalida! Tente novamente.\n" RESET);
        }
    } while (option != 0);

    return 0;
}