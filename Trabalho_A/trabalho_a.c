#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100 
#define ARQUIVO_CSV "playlist.csv"

typedef struct { 
    int id;
    char titulo[50];
    char artist[50]; // mantido o padrão do enunciado (artista)
    int duracao;
    char genero[30];
} Musica;

typedef struct {
    Musica dados[MAX]; 
    int quantidade;
} Playlist;

// Função auxiliar para limpar o buffer do teclado de forma segura
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Retorna o índice da música na playlist se achar, ou -1 se não achar
int buscarPorId(Playlist *p, int id) {
    for (int i = 0; i < p->quantidade; i++) {
        if (p->dados[i].id == id) {
            return i;
        }
    }
    return -1;
}

void inserir(Playlist *p) {
    if (p->quantidade >= MAX) {
        printf("Erro: A playlist está cheia! (Máximo %d músicas)\n", MAX);
        return;
    }
    
    Musica novaMusica;
    printf("Digite o ID único da música: ");
    if (scanf("%d", &novaMusica.id) != 1) {
        printf("Entrada inválida!\n");
        limparBuffer();
        return;
    }
    limparBuffer();

    // Validação de chave duplicada
    if (buscarPorId(p, novaMusica.id) != -1) {
        printf("Erro: O ID %d já foi cadastrado!\n", novaMusica.id);
        return;
    }
    
    printf("Digite o título da música: ");
    fgets(novaMusica.titulo, sizeof(novaMusica.titulo), stdin);
    novaMusica.titulo[strcspn(novaMusica.titulo, "\n")] = '\0';
    
    printf("Digite o nome do artista: ");
    fgets(novaMusica.artist, sizeof(novaMusica.artist), stdin);
    novaMusica.artist[strcspn(novaMusica.artist, "\n")] = '\0';
    
    printf("Digite o tempo de duração da música (em segundos): ");
    if (scanf("%d", &novaMusica.duracao) != 1 || novaMusica.duracao < 0) {
        printf("Duração inválida!\n");
        limparBuffer();
        return;
    }
    limparBuffer();
    
    printf("Digite o gênero da música: ");
    fgets(novaMusica.genero, sizeof(novaMusica.genero), stdin);
    novaMusica.genero[strcspn(novaMusica.genero, "\n")] = '\0';
    
    p->dados[p->quantidade] = novaMusica;
    p->quantidade++;
    
    printf("\nMúsica registrada com sucesso!\n");
}

void buscar(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia.\n");
        return;
    }

    int id;
    printf("Digite o ID da música que deseja buscar: ");
    scanf("%d", &id);
    limparBuffer();

    int indice = buscarPorId(p, id);
    if (indice == -1) {
        printf("Música com ID %d não encontrada.\n", id);
    } else {
        printf("\n--- MÚSICA ENCONTRADA ---\n");
        printf("ID: %d\n", p->dados[indice].id);
        printf("Título: %s\n", p->dados[indice].titulo);
        printf("Artista: %s\n", p->dados[indice].artist);
        printf("Duração: %d segundos (%d:%02d)\n", p->dados[indice].duracao, p->dados[indice].duracao / 60, p->dados[indice].duracao % 60);
        printf("Gênero: %s\n", p->dados[indice].genero);
        printf("-------------------------\n");
    }
}

void editar(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia.\n");
        return;
    }

    int id;
    printf("Digite o ID da música que deseja editar: ");
    scanf("%d", &id);
    limparBuffer();

    int idx = buscarPorId(p, id);
    if (idx == -1) {
        printf("Música com ID %d não encontrada.\n", id);
        return;
    }

    printf("\nEditando campos (ID não pode ser alterado):\n");
    
    printf("Novo título (Atual: %s): ", p->dados[idx].titulo);
    fgets(p->dados[idx].titulo, sizeof(p->dados[idx].titulo), stdin);
    p->dados[idx].titulo[strcspn(p->dados[idx].titulo, "\n")] = '\0';

    printf("Novo artista (Atual: %s): ", p->dados[idx].artist);
    fgets(p->dados[idx].artist, sizeof(p->dados[idx].artist), stdin);
    p->dados[idx].artist[strcspn(p->dados[idx].artist, "\n")] = '\0';

    printf("Nova duração em segundos (Atual: %d): ", p->dados[idx].duracao);
    scanf("%d", &p->dados[idx].duracao);
    limparBuffer();

    printf("Novo gênero (Atual: %s): ", p->dados[idx].genero);
    fgets(p->dados[idx].genero, sizeof(p->dados[idx].genero), stdin);
    p->dados[idx].genero[strcspn(p->dados[idx].genero, "\n")] = '\0';

    printf("\nMúsica editada com sucesso!\n");
}

void excluir(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia.\n");
        return;
    }

    int id;
    printf("Digite o ID da música que deseja excluir: ");
    scanf("%d", &id);
    limparBuffer();

    int idx = buscarPorId(p, id);
    if (idx == -1) {
        printf("Música com ID %d não encontrada.\n", id);
        return;
    }

    // Funcionalidade Extra: Confirmação antes da exclusão
    char conf;
    printf("Tem certeza que deseja excluir a música \"%s\"? (S/N): ", p->dados[idx].titulo);
    scanf("%c", &conf);
    limparBuffer();

    if (conf == 'S' || conf == 's') {
        // Desloca os elementos posteriores para preencher o espaço vazio
        for (int i = idx; i < p->quantidade - 1; i++) {
            p->dados[i] = p->dados[i + 1];
        }
        p->quantidade--;
        printf("Música excluída com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

void listar(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia.\n");
        return;
    }

    printf("\n======================= PLAYLIST ATUAL =======================\n");
    printf("%-5s | %-25s | %-20s | %-7s | %-15s\n", "ID", "Título", "Artista", "Dur(s)", "Gênero");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < p->quantidade; i++) {
        printf("%-5d | %-25s | %-20s | %-7d | %-15s\n", 
               p->dados[i].id, p->dados[i].titulo, p->dados[i].artist, p->dados[i].duracao, p->dados[i].genero);
    }
    printf("==============================================================\n");
}

void salvarCSV(Playlist *p) {
    FILE *f = fopen(ARQUIVO_CSV, "w");
    if (f == NULL) {
        printf("Erro ao abrir arquivo para salvar!\n");
        return;
    }

    // Cabeçalho do CSV
    fprintf(f, "id;titulo;artista;duracao;genero\n");

    for (int i = 0; i < p->quantidade; i++) {
        fprintf(f, "%d;%s;%s;%d;%s\n", 
                p->dados[i].id, p->dados[i].titulo, p->dados[i].artist, p->dados[i].duracao, p->dados[i].genero);
    }

    fclose(f);
    printf("Dados salvos em \"%s\" com sucesso!\n", ARQUIVO_CSV);
}

void carregarCSV(Playlist *p) {
    FILE *f = fopen(ARQUIVO_CSV, "r");
    if (f == NULL) {
        printf("Nenhum arquivo de dados prévio encontrado (\"%s\"). Iniciando playlist limpa.\n", ARQUIVO_CSV);
        return;
    }

    char linha[200];
    // Pular a linha do cabeçalho
    if (fgets(linha, sizeof(linha), f) == NULL) {
        fclose(f);
        return;
    }

    p->quantidade = 0;
    while (fgets(linha, sizeof(linha), f) != NULL && p->quantidade < MAX) {
        Musica m;
        // Removendo a quebra de linha que o fgets traz
        linha[strcspn(linha, "\n")] = '\0';

        // Parsing da linha usando strtok com o delimitador ';'
        char *token = strtok(linha, ";");
        if (token == NULL) continue;
        m.id = atoi(token);

        token = strtok(NULL, ";");
        if (token == NULL) continue;
        strcpy(m.titulo, token);

        token = strtok(NULL, ";");
        if (token == NULL) continue;
        strcpy(m.artist, token);

        token = strtok(NULL, ";");
        if (token == NULL) continue;
        m.duracao = atoi(token);

        token = strtok(NULL, ";");
        if (token == NULL) continue;
        strcpy(m.genero, token);

        p->dados[p->quantidade] = m;
        p->quantidade++;
    }

    fclose(f);
    printf("Carregados %d registros de \"%s\" com sucesso!\n", p->quantidade, ARQUIVO_CSV);
}

// Funcionalidade Extra: Estatísticas da Playlist
void exibirEstatisticas(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia. Sem estatísticas disponíveis.\n");
        return;
    }

    int totalDuracao = 0;
    for (int i = 0; i < p->quantidade; i++) {
        totalDuracao += p->dados[i].duracao;
    }

    double media = (double)totalDuracao / p->quantidade;

    printf("\n===== ESTATÍSTICAS DA PLAYLIST =====\n");
    printf("Total de músicas cadastradas: %d\n", p->quantidade);
    printf("Tempo total da playlist: %d segundos (%d:%02d)\n", totalDuracao, totalDuracao / 3600, (totalDuracao % 3600) / 60);
    printf("Duração média por música: %.1f segundos\n", media);
    printf("Espaço disponível no vetor: %d\n", MAX - p->quantidade);
    printf("====================================\n");
}

// Funcionalidade Extra: Busca parcial por título
void buscaParcial(Playlist *p) {
    if (p->quantidade == 0) {
        printf("A playlist está vazia.\n");
        return;
    }

    char termo[50];
    printf("Digite parte do título da música para buscar: ");
    fgets(termo, sizeof(termo), stdin);
    termo[strcspn(termo, "\n")] = '\0';

    printf("\n--- RESULTADOS DA BUSCA PARCIAL ---\n");
    int encontrados = 0;
    for (int i = 0; i < p->quantidade; i++) {
        // strstr busca uma substring dentro de outra string
        if (strstr(p->dados[i].titulo, termo) != NULL) {
            printf("[%d] %s - %s (%ds)\n", p->dados[i].id, p->dados[i].titulo, p->dados[i].artist, p->dados[i].duracao);
            encontrados++;
        }
    }
    if (encontrados == 0) {
        printf("Nenhuma música corresponde ao termo digitado.\n");
    }
    printf("------------------------------------\n");
}

void menu() {
    printf("\n___________________________________\n");
    printf("|        PLAYLIST DE MÚSICAS       |\n");
    printf("|_________________________________|\n");
    printf("|   1 - REGISTRAR MÚSICA          |\n");
    printf("|   2 - PESQUISAR MÚSICA (ID)     |\n");
    printf("|   3 - EDITAR REGISTRO           |\n");
    printf("|   4 - EXCLUIR MÚSICA            |\n");
    printf("|   5 - LISTAR MÚSICAS            |\n");
    printf("|   6 - SALVAR CSV                |\n");
    printf("|   7 - CARREGAR CSV              |\n");
    printf("|   9 - EXIBIR ESTATÍSTICAS (Ext) |\n");
    printf("|  10 - BUSCA PARCIAL TÍTULO(Ext) |\n");
    printf("|   8 - SAIR                      |\n");
    printf("|_________________________________|\n\n");
}

int main() {
    Playlist minhaPlaylist;
    minhaPlaylist.quantidade = 0;
    int menuOp = 0;

    // Carrega automaticamente o CSV ao iniciar (conforme solicitado na especificação do roteiro)
    carregarCSV(&minhaPlaylist);
    printf("\nPressione ENTER para continuar...");
    limparBuffer();

    do {
        menu();
        printf("Escolha a opção desejada: ");
        if (scanf("%d", &menuOp) != 1) {
            printf("Opção inválida! Digite um número.\n");
            limparBuffer();
            continue;
        }
        limparBuffer(); // limpa o \n do enter

        switch (menuOp) {
            case 1:
                inserir(&minhaPlaylist);
                break;
            case 2:
                buscar(&minhaPlaylist);
                break;
            case 3:
                editar(&minhaPlaylist);
                break;
            case 4:
                excluir(&minhaPlaylist);
                break;
            case 5:
                listar(&minhaPlaylist);
                break;
            case 6:
                salvarCSV(&minhaPlaylist);
                break;
            case 7:
                carregarCSV(&minhaPlaylist);
                break;
            case 9:
                exibirEstatisticas(&minhaPlaylist);
                break;
            case 10:
                buscaParcial(&minhaPlaylist);
                break;
            case 8:
                // Auto-salvamento opcional/recomendado ao sair
                salvarCSV(&minhaPlaylist);
                printf("\nSistema finalizado com sucesso!\n");
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
                break;
        }
    } while (menuOp != 8);

    return 0;
}
