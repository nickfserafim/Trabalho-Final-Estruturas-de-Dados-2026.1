#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// 1. DEFINIÇÃO DA STRUCT DO REGISTRO (Campos exigidos)
// ============================================================================
typedef struct Node {
    int senha;
    char tipo;          // 'N' para Normal, 'P' para Preferencial
    char horario[20];
    struct Node* proximo;
} Node;

// Estrutura para controle da Fila Normal (Encadeada Simples)
typedef struct {
    Node* inicio;
    Node* fim;
} FilaNormal;

// Estrutura para controle da Fila Preferencial (Circular Encadeada)
typedef struct {
    Node* inicio;
    Node* fim;
    int quantidade; // Exigido para controle da circular
} FilaCircularPreferencial;

// Estatísticas globais (Funcionalidade Extra)
int total_atendidos = 0;
int proporcao_normal = 2; // Regra configurável: 2 Normais para 1 Preferencial
int chamadas_seguidas_normal = 0;

/* Variáveis extras para o relatório estatístico (tempo médio de espera).
   Guardo a soma dos minutos de espera de cada senha atendida e a quantidade
   de cada tipo atendido separadamente, assim consigo montar um relatório
   mais completo na hora de exibir (item "relatorios estatisticos" do extra). */
int total_atendidos_normal = 0;
int total_atendidos_preferencial = 0;
long soma_minutos_espera = 0; // soma de (hora_atendimento - hora_geracao) em minutos

// ============================================================================
// 2. FUNÇÕES DE INICIALIZAÇÃO
// ============================================================================
void inicializarFilaNormal(FilaNormal* f) {
    f->inicio = NULL;
    f->fim = NULL;
}

void inicializarFilaCircular(FilaCircularPreferencial* f) {
    f->inicio = NULL;
    f->fim = NULL;
    f->quantidade = 0;
}

// Auxiliar para obter o horário atual formatado
void obterHorarioAtual(char* buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "%02d:%02d", tm.tm_hour, tm.tm_min);
}

/* Auxiliar para calcular quantos minutos se passaram entre o horario em que
   a senha foi gerada e o horario atual (usado so para o relatorio de tempo
   medio de espera). Como o horario eh salvo so como HH:MM, faco a conta
   "na mao" convertendo tudo para minutos do dia. Nao trata virada de dia
   (ex: gerou 23:50 e atendeu 00:10) porque para o uso do trabalho isso
   nao deve acontecer, mas deixei o comentario aqui pra registrar a limitacao. */
int calcularMinutosEspera(char* horarioGeracao) {
    int hG, mG, hA, mA;
    char bufferAtual[20];
    obterHorarioAtual(bufferAtual);

    sscanf(horarioGeracao, "%d:%d", &hG, &mG);
    sscanf(bufferAtual, "%d:%d", &hA, &mA);

    int minutosGeracao = hG * 60 + mG;
    int minutosAtual = hA * 60 + mA;

    int diferenca = minutosAtual - minutosGeracao;
    if (diferenca < 0) diferenca = 0; // protecao basica, nao deveria ser negativo

    return diferenca;
}

// ============================================================================
// 3. OPERAÇÕES DE ENFILEIRAR (Trata fila cheia/vazia dinamicamente)
// ============================================================================
void enfileirarNormal(FilaNormal* f, int senha, char* horario) {
    Node* novo = (Node*)malloc(sizeof(Node));
    if (novo == NULL) {
        printf("[Erro] Memoria cheia! Nao foi possivel gerar senha.\n");
        return;
    }
    novo->senha = senha;
    novo->tipo = 'N';
    strcpy(novo->horario, horario);
    novo->proximo = NULL;

    if (f->inicio == NULL) { // Fila Vazia
        f->inicio = novo;
    } else {
        f->fim->proximo = novo;
    }
    f->fim = novo;
    printf("Senha Normal N%d gerada com sucesso!\n", senha);
}

void enfileirarPreferencial(FilaCircularPreferencial* f, int senha, char* horario) {
    Node* novo = (Node*)malloc(sizeof(Node));
    if (novo == NULL) {
        printf("[Erro] Memoria cheia! Nao foi possivel gerar senha.\n");
        return;
    }
    novo->senha = senha;
    novo->tipo = 'P';
    strcpy(novo->horario, horario);

    if (f->inicio == NULL) { // Caso: Fila possui apenas um elemento
        f->inicio = novo;
        f->fim = novo;
        novo->proximo = novo; // Aponta para si mesmo (Circularidade)
    } else {
        novo->proximo = f->inicio; // Novo nó aponta para o começo
        f->fim->proximo = novo;    // Antigo fim aponta para o novo
        f->fim = novo;             // Atualiza o ponteiro de fim
    }
    f->quantidade++;
    printf("Senha Preferencial P%d gerada com sucesso!\n", senha);
}

// ============================================================================
// 4. OPERAÇÕES DE LISTAGEM / EXIBIÇÃO
// ============================================================================
void listarFilas(FilaNormal* fn, FilaCircularPreferencial* fp) {
    printf("\n=== FILA NORMAL ===\n");
    if (fn->inicio == NULL) {
        printf("Fila vazia.\n");
    } else {
        Node* atual = fn->inicio;
        while (atual != NULL) {
            printf("[N%d | %s] -> ", atual->senha, atual->horario);
            atual = atual->proximo;
        }
        printf("NULL\n");
    }

    printf("\n=== FILA PREFERENCIAL (CIRCULAR) ===\n");
    if (fp->inicio == NULL) {
        printf("Fila vazia.\n");
    } else {
        Node* atual = fp->inicio;
        int cont = 0;
        // Laço controlado pela quantidade para garantir segurança na fila circular
        while (cont < fp->quantidade) {
            printf("[P%d | %s] -> ", atual->senha, atual->horario);
            atual = atual->proximo;
            cont++;
        }
        printf("(Volta para P%d)\n", fp->inicio->senha);
    }
}

// ============================================================================
// 5. TODO 1 - DESENFILEIRAR E REGRA DE ATENDIMENTO (CHAMAR PROXIMO)
// ============================================================================
/* Essa eh a funcao principal do "atender". A regra que o enunciado pede eh:
   - atende ate "proporcao_normal" senhas normais em sequencia
   - quando bater esse limite (ou a fila normal estiver vazia), tenta atender
     um preferencial e zera o contador de "seguidas"
   - se nao tiver preferencial tambem, atende normal mesmo assim (se tiver)
   Tratei fila vazia nos dois lados e cheia ja eh tratada no malloc das
   funcoes de enfileirar, entao aqui so falta a remocao. */
void chamarProximo(FilaNormal* fn, FilaCircularPreferencial* fp) {

    // Caso 1: as duas filas estao vazias, nao tem o que fazer
    if (fn->inicio == NULL && fp->inicio == NULL) {
        printf("\nNao ha ninguem nas filas para ser atendido.\n");
        return;
    }

    int atenderPreferencial = 0;

    /* Decide se deve atender preferencial agora:
       - se ja bateu a proporcao configurada de normais seguidos
       - ou se a fila normal esta vazia (nesse caso so resta preferencial)
       Mas so faz sentido atender preferencial se ela tiver gente, claro. */
    if (fp->inicio != NULL && (chamadas_seguidas_normal >= proporcao_normal || fn->inicio == NULL)) {
        atenderPreferencial = 1;
    }

    if (atenderPreferencial) {
        // ---------- ATENDER PREFERENCIAL (remover da fila circular) ----------
        Node* removido = fp->inicio;

        int minutos = calcularMinutosEspera(removido->horario);
        soma_minutos_espera += minutos;
        total_atendidos_preferencial++;
        total_atendidos++;

        printf("\n>> Atendendo senha PREFERENCIAL P%d (gerada %s, esperou %d min)\n",
               removido->senha, removido->horario, minutos);

        if (fp->quantidade == 1) {
            // Unico elemento da fila circular: ao remover, fila fica vazia
            fp->inicio = NULL;
            fp->fim = NULL;
        } else {
            // Fim aponta direto pro segundo elemento, "pulando" o que sai
            fp->inicio = removido->proximo;
            fp->fim->proximo = fp->inicio;
        }
        fp->quantidade--;
        free(removido);

        chamadas_seguidas_normal = 0; // zera o contador depois de atender preferencial

    } else {
        // ---------- ATENDER NORMAL (remover da fila encadeada simples) ----------
        if (fn->inicio == NULL) {
            // Nao deveria entrar aqui por causa do "Caso 1" acima, mas fica
            // a guarda de seguranca caso a logica mude no futuro.
            printf("\nFila normal esta vazia.\n");
            return;
        }

        Node* removido = fn->inicio;

        int minutos = calcularMinutosEspera(removido->horario);
        soma_minutos_espera += minutos;
        total_atendidos_normal++;
        total_atendidos++;

        printf("\n>> Atendendo senha NORMAL N%d (gerada %s, esperou %d min)\n",
               removido->senha, removido->horario, minutos);

        fn->inicio = removido->proximo;
        if (fn->inicio == NULL) {
            // Removeu o ultimo elemento da fila, entao fim tambem vira NULL
            fn->fim = NULL;
        }
        free(removido);

        chamadas_seguidas_normal++;
    }
}

// ============================================================================
// 6. TODO 2 - CONSULTAR PROXIMO (ESPIAR SEM REMOVER)
// ============================================================================
/* Funcao simples so pra "espiar" quem seria o proximo de cada fila, sem
   tirar ninguem dela. Util pro atendente conferir antes de chamar. */
void espiarProximos(FilaNormal* fn, FilaCircularPreferencial* fp) {
    printf("\n--- PROXIMOS DA FILA (sem remover) ---\n");

    if (fn->inicio == NULL) {
        printf("Fila Normal: vazia.\n");
    } else {
        printf("Fila Normal: proximo eh N%d (gerada %s)\n",
               fn->inicio->senha, fn->inicio->horario);
    }

    if (fp->inicio == NULL) {
        printf("Fila Preferencial: vazia.\n");
    } else {
        printf("Fila Preferencial: proximo eh P%d (gerada %s)\n",
               fp->inicio->senha, fp->inicio->horario);
    }
}

// ============================================================================
// 7. TODO 3 - BUSCAR E CANCELAR SENHA
// ============================================================================
/* Procura a senha nas duas filas. Se achar, pede confirmacao (extra pedido
   no enunciado: "solicitar confirmacao antes do cancelamento") e só remove
   se o usuario confirmar com 's'. Cobri os 3 casos de remocao na lista
   simples (inicio, meio/fim, unico elemento) e na circular (unico elemento,
   inicio, qualquer outra posicao). */
void buscarCancelarSenha(FilaNormal* fn, FilaCircularPreferencial* fp, int senhaBuscada) {

    // ---------- Procura primeiro na fila normal ----------
    Node* atual = fn->inicio;
    Node* anterior = NULL;

    while (atual != NULL) {
        if (atual->senha == senhaBuscada) {
            printf("\nSenha N%d encontrada na fila NORMAL (gerada %s).\n",
                   atual->senha, atual->horario);
            printf("Confirma o cancelamento? (s/n): ");

            char confirmacao;
            scanf(" %c", &confirmacao);

            if (confirmacao == 's' || confirmacao == 'S') {
                if (anterior == NULL) {
                    // Era o primeiro da fila
                    fn->inicio = atual->proximo;
                } else {
                    anterior->proximo = atual->proximo;
                }
                if (atual == fn->fim) {
                    // Era o ultimo da fila, ajusta o fim
                    fn->fim = anterior;
                }
                free(atual);
                printf("Senha N%d cancelada com sucesso.\n", senhaBuscada);
            } else {
                printf("Cancelamento abortado pelo usuario.\n");
            }
            return; // achou e tratou, nao precisa continuar procurando
        }
        anterior = atual;
        atual = atual->proximo;
    }

    // ---------- Se nao achou na normal, procura na preferencial (circular) ----------
    if (fp->inicio != NULL) {
        Node* atualP = fp->inicio;
        Node* anteriorP = fp->fim; // no circular, "anterior" do inicio eh o fim
        int cont = 0;

        while (cont < fp->quantidade) {
            if (atualP->senha == senhaBuscada) {
                printf("\nSenha P%d encontrada na fila PREFERENCIAL (gerada %s).\n",
                       atualP->senha, atualP->horario);
                printf("Confirma o cancelamento? (s/n): ");

                char confirmacao;
                scanf(" %c", &confirmacao);

                if (confirmacao == 's' || confirmacao == 'S') {
                    if (fp->quantidade == 1) {
                        // Unico elemento, fila fica vazia depois de remover
                        fp->inicio = NULL;
                        fp->fim = NULL;
                    } else {
                        anteriorP->proximo = atualP->proximo;
                        if (atualP == fp->inicio) {
                            fp->inicio = atualP->proximo;
                        }
                        if (atualP == fp->fim) {
                            fp->fim = anteriorP;
                        }
                    }
                    fp->quantidade--;
                    free(atualP);
                    printf("Senha P%d cancelada com sucesso.\n", senhaBuscada);
                } else {
                    printf("Cancelamento abortado pelo usuario.\n");
                }
                return;
            }
            anteriorP = atualP;
            atualP = atualP->proximo;
            cont++;
        }
    }

    // Se chegou aqui, nao achou em nenhuma das duas filas
    printf("\nSenha %d nao foi encontrada em nenhuma fila.\n", senhaBuscada);
}

// ============================================================================
// 8. FUNCIONALIDADE EXTRA - RELATORIO ESTATISTICO
// ============================================================================
/* Mostra quantos atendimentos foram feitos de cada tipo e o tempo medio de
   espera (em minutos) considerando todos os atendidos. Evito divisao por
   zero quando ainda nao atendeu nenhuma senha. */
void exibirRelatorio() {
    printf("\n========== RELATORIO ESTATISTICO ==========\n");
    printf("Total atendidos (Normal):        %d\n", total_atendidos_normal);
    printf("Total atendidos (Preferencial):  %d\n", total_atendidos_preferencial);
    printf("Total atendidos (Geral):         %d\n", total_atendidos);

    if (total_atendidos > 0) {
        double media = (double) soma_minutos_espera / total_atendidos;
        printf("Tempo medio de espera:           %.2f min\n", media);
    } else {
        printf("Tempo medio de espera:           N/A (nenhum atendimento ainda)\n");
    }
    printf("=============================================\n");
}

// ============================================================================
// 9. FUNCIONALIDADE EXTRA - CONFIGURAR PROPORCAO PELO MENU
// ============================================================================
/* So pede um numero novo pro usuario e valida que seja maior que zero,
   senao a regra de "chamadas_seguidas_normal >= proporcao_normal" nunca
   bateria e o preferencial nunca seria chamado (ou batia toda hora se
   fosse 0). */
void configurarProporcao() {
    int novaProporcao;
    printf("\nProporcao atual: %d normal(is) para 1 preferencial.\n", proporcao_normal);
    printf("Digite a nova proporcao (numero inteiro maior que 0): ");

    // Mesma protecao usada no menu principal: se digitar algo que nao
    // eh numero, limpa o buffer e trata como valor invalido.
    if (scanf("%d", &novaProporcao) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        printf("Entrada invalida! A proporcao precisa ser um numero. Mantendo o valor antigo (%d).\n", proporcao_normal);
        return;
    }

    if (novaProporcao <= 0) {
        printf("Valor invalido! A proporcao precisa ser maior que zero. Mantendo o valor antigo (%d).\n", proporcao_normal);
        return;
    }

    proporcao_normal = novaProporcao;
    chamadas_seguidas_normal = 0; // zera o contador pra nao ficar com lixo de uma config antiga
    printf("Proporcao atualizada para %d normal(is) para 1 preferencial.\n", proporcao_normal);
}

// ============================================================================
// 10. TODO 4 - PERSISTENCIA EM CSV (SALVAR E CARREGAR)
// ============================================================================
/* Salva as duas filas num unico arquivo CSV, no formato "senha;tipo;horario".
   Coloco um cabecalho na primeira linha so pra ficar mais facil de abrir e
   entender o arquivo no Excel/Sheets, e essa linha eh ignorada no momento
   de carregar (como pede o enunciado). */
void salvarCSV(FilaNormal* fn, FilaCircularPreferencial* fp, const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL) {
        printf("\n[Erro] Nao foi possivel abrir o arquivo para salvar.\n");
        return;
    }

    fprintf(arquivo, "senha;tipo;horario\n"); // cabecalho

    Node* atual = fn->inicio;
    while (atual != NULL) {
        fprintf(arquivo, "%d;%c;%s\n", atual->senha, atual->tipo, atual->horario);
        atual = atual->proximo;
    }

    if (fp->inicio != NULL) {
        Node* atualP = fp->inicio;
        int cont = 0;
        while (cont < fp->quantidade) {
            fprintf(arquivo, "%d;%c;%s\n", atualP->senha, atualP->tipo, atualP->horario);
            atualP = atualP->proximo;
            cont++;
        }
    }

    fclose(arquivo);
    printf("\nFilas salvas com sucesso em '%s'.\n", nomeArquivo);
}

/* Carrega o CSV e reconstroi as filas. Pra evitar duplicatas (pedido no
   enunciado), checo antes se a senha ja existe em alguma das duas filas
   antes de inserir; se ja existir, simplesmente ignoro aquela linha.
   Tambem atualizo o "contador_senha" externo (recebido por ponteiro) pra
   continuar a numeracao de onde o arquivo parou, evitando gerar senha
   repetida depois de carregar. */
int senhaJaExiste(FilaNormal* fn, FilaCircularPreferencial* fp, int senha) {
    Node* atual = fn->inicio;
    while (atual != NULL) {
        if (atual->senha == senha) return 1;
        atual = atual->proximo;
    }

    if (fp->inicio != NULL) {
        Node* atualP = fp->inicio;
        int cont = 0;
        while (cont < fp->quantidade) {
            if (atualP->senha == senha) return 1;
            atualP = atualP->proximo;
            cont++;
        }
    }
    return 0;
}

void carregarCSV(FilaNormal* fn, FilaCircularPreferencial* fp, const char* nomeArquivo, int* contadorSenha) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        printf("\n[Erro] Arquivo '%s' nao encontrado.\n", nomeArquivo);
        return;
    }

    char linha[100];
    int primeiraLinha = 1;
    int carregados = 0;
    int ignoradosDuplicados = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {

        if (primeiraLinha) {
            // pula o cabecalho "senha;tipo;horario"
            primeiraLinha = 0;
            continue;
        }

        // remove o '\n' que o fgets deixa no final da linha, se tiver
        linha[strcspn(linha, "\n")] = '\0';

        if (strlen(linha) == 0) {
            continue; // ignora linha vazia (ex: linha final do arquivo)
        }

        int senha;
        char tipo;
        char horario[20];

        // formato esperado: senha;tipo;horario  ->  ex: 3;N;14:25
        int lidos = sscanf(linha, "%d;%c;%19[^;\n]", &senha, &tipo, horario);

        if (lidos != 3) {
            printf("Linha invalida ignorada: %s\n", linha);
            continue;
        }

        if (senhaJaExiste(fn, fp, senha)) {
            ignoradosDuplicados++;
            continue;
        }

        if (tipo == 'N') {
            enfileirarNormal(fn, senha, horario);
        } else if (tipo == 'P') {
            enfileirarPreferencial(fp, senha, horario);
        } else {
            printf("Tipo desconhecido na linha, ignorada: %s\n", linha);
            continue;
        }

        carregados++;

        // mantem o contador de senha sempre maior que a maior senha carregada
        if (senha >= *contadorSenha) {
            *contadorSenha = senha + 1;
        }
    }

    fclose(arquivo);
    printf("\nCarregamento concluido: %d senha(s) carregada(s), %d duplicada(s) ignorada(s).\n",
           carregados, ignoradosDuplicados);
}

// ============================================================================
// 11. TODO 5 - LIMPEZA DE MEMORIA AO SAIR
// ============================================================================
/* Libera todos os nos que ainda restarem nas duas filas quando o programa
   for fechado, pra nao deixar memoria "presa" (leak). Na fila circular
   preciso guardar uma referencia pro "proximo" antes de fazer o free,
   porque depois de liberar o no eu nao posso mais ler o campo dele. */
void liberarMemoria(FilaNormal* fn, FilaCircularPreferencial* fp) {
    // ---------- Libera a fila normal ----------
    Node* atual = fn->inicio;
    while (atual != NULL) {
        Node* proximoTemp = atual->proximo;
        free(atual);
        atual = proximoTemp;
    }
    fn->inicio = NULL;
    fn->fim = NULL;

    // ---------- Libera a fila preferencial (circular) ----------
    if (fp->inicio != NULL) {
        Node* atualP = fp->inicio;
        int cont = 0;
        int total = fp->quantidade;

        while (cont < total) {
            Node* proximoTemp = atualP->proximo;
            free(atualP);
            atualP = proximoTemp;
            cont++;
        }
    }
    fp->inicio = NULL;
    fp->fim = NULL;
    fp->quantidade = 0;

    printf("\nMemoria liberada com sucesso. Encerrando o sistema...\n");
}

// ============================================================================
// 12. MENU INTERATIVO E FLUXO PRINCIPAL
// ============================================================================
int main() {
    FilaNormal filaNormal;
    FilaCircularPreferencial filaPref;

    inicializarFilaNormal(&filaNormal);
    inicializarFilaCircular(&filaPref);

    int opcao;
    int contador_senha = 1; // Auto-incremento para testes rápidos
    char horario_buffer[20];

    do {
        printf("\n=================================");
        printf("\nSISTEMA DE ATENDIMENTO BANCARIO");
        printf("\n=================================");
        printf("\n1. Gerar Senha Normal");
        printf("\n2. Gerar Senha Preferencial");
        printf("\n3. Chamar Proximo (Atender)");
        printf("\n4. Exibir as Duas Filas");
        printf("\n5. Buscar / Cancelar Senha");
        printf("\n6. Configurar Proporcao (Atual: %d por 1)", proporcao_normal);
        printf("\n7. Salvar Filas em CSV");
        printf("\n8. Carregar Filas de CSV");
        printf("\n9. Exibir Relatorio Estatistico");
        printf("\n10. Consultar Proximos (sem remover)");
        printf("\n0. Sair");
        printf("\nEscolha uma opcao: ");

        /* Protecao contra entrada invalida (ex: digitar uma letra onde
           era esperado um numero). Sem isso, o scanf("%d") falha, NAO
           consome o caractere digitado e a variavel "opcao" fica com o
           valor antigo, fazendo o programa repetir a mesma opcao pra
           sempre (loop infinito). Aqui, se a leitura falhar, eu limpo
           o buffer ate a quebra de linha e forco a opcao para um valor
           invalido, que vai cair no "default" do switch. */
        if (scanf("%d", &opcao) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            opcao = -1;
        }

        switch (opcao) {
            case 1:
                obterHorarioAtual(horario_buffer);
                enfileirarNormal(&filaNormal, contador_senha++, horario_buffer);
                break;
            case 2:
                obterHorarioAtual(horario_buffer);
                enfileirarPreferencial(&filaPref, contador_senha++, horario_buffer);
                break;
            case 3:
                chamarProximo(&filaNormal, &filaPref);
                break;
            case 4:
                listarFilas(&filaNormal, &filaPref);
                break;
            case 5: {
                int senhaBusca;
                printf("\nDigite a senha que deseja buscar/cancelar: ");

                if (scanf("%d", &senhaBusca) != 1) {
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("Entrada invalida! Digite apenas o numero da senha.\n");
                    break;
                }
                buscarCancelarSenha(&filaNormal, &filaPref, senhaBusca);
                break;
            }
            case 6:
                configurarProporcao();
                break;
            case 7:
                salvarCSV(&filaNormal, &filaPref, "filas_atendimento.csv");
                break;
            case 8:
                carregarCSV(&filaNormal, &filaPref, "filas_atendimento.csv", &contador_senha);
                break;
            case 9:
                exibirRelatorio();
                break;
            case 10:
                espiarProximos(&filaNormal, &filaPref);
                break;
            case 0:
                liberarMemoria(&filaNormal, &filaPref);
                printf("Saindo do sistema...\n");
                break;
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
        }
    } while (opcao != 0);

    return 0;
}