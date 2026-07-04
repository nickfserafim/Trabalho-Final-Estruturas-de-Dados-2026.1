# Trabalho Final Estruturas de Dados 2026.1
Escopo do Relatório: Análise de três softwares desenvolvidos de forma isolada, cada um explorando uma estrutura de dados linear clássica e enfrentando desafios particulares de persistência em disco.

Divisão dos Projetos:

Trabalho A (Gerenciador de Playlist): Desenvolvido por Poliana e Nicolas.

Trabalho B (Histórico de Navegador): Desenvolvido por Isaque.

Trabalho C (Fila de Atendimento Bancário): Desenvolvido por Pedro e Ian.

1. Abordagens Técnicas e Estruturas de Dados por Projeto
Como os projetos foram conduzidos de forma estritamente separada, cada um adotou uma topologia de memória adequada ao seu problema de negócio:

Trabalho A: Gerenciador de Playlist (Lista Estática)
Abordagem: Utilização de um vetor sequencial estático (struct encapsulada em um array com #define MAX).

Justificativa: Como o enunciado previa uma lista de reprodução com limite previsível de faixas, a alocação contígua em memória facilitou o acesso direto via índices. As operações de busca e exibição foram resolvidas com laços simples e aritmética de ponteiros nativa do vetor.

Trabalho B: Histórico de Navegador (Pilha Dinâmica)
Abordagem: Implementação de uma Pilha Dinâmica Encadeada (estruturada em nós/Node alocados em tempo de execução).

Justificativa: O modelo LIFO (Last In, First Out) é o padrão para o comportamento de navegadores web (o botão "Voltar"). A alocação dinâmica foi essencial aqui para que o histórico pudesse crescer e reduzir de tamanho de forma flexível a cada nova URL visitada (push) ou retornada (pop), limpando a memória com free().

Trabalho C: Atendimento Bancário (Fila Dinâmica/Estática)
Abordagem: Implementação do conceito de Fila sob a ótica FIFO (First In, First Out).

Justificativa: Para simular o fluxo de clientes em uma agência bancária, a estrutura de fila é mandatória para garantir a ordem justa de chegada (o primeiro que emite a senha é obrigatoriamente o primeiro a ser chamado pelo painel).

2. A Dificuldade Compartilhada: O Desafio do CSV
Apesar de operarem em códigos e sistemas totalmente separados, todas as três frentes de desenvolvimento encontraram seu maior obstáculo técnico na implementação da persistência em arquivos CSV. Cada estrutura linear impôs uma barreira lógica diferente ao interagir com o disco:

O Desafio da Lista Estática (Trabalho A)
No vetor, a gravação é direta, mas o problema crítico surge na remoção de elementos antes de salvar. Para que o arquivo CSV não fique com lacunas ou registros "fantasmas", a dupla precisou garantir que a exclusão de uma música fizesse o deslocamento (shift) de todos os elementos subsequentes na memória antes de disparar a rotina de escrita no arquivo.

O Desafio da Pilha Dinâmica (Trabalho B)
O obstáculo no histórico do navegador foi o mais complexo devido à natureza da estrutura. Ao salvar a pilha, a varredura ocorre do Topo para a Base para gerar o arquivo. No entanto, ao ler esse arquivo de forma linear de cima para baixo no fechamento/abertura do programa, a inserção direta com push inverteria completamente a ordem do histórico.
Para contornar isso de forma isolada, o desenvolvedor precisou adotar uma estratégia de buffer: carregar as linhas do arquivo em um vetor temporário e, em seguida, fazer um laço reverso (for decrescente) para inserir os nós na pilha a partir da base original, preservando o estado real do navegador.

O Desafio da Fila de Banco (Trabalho C)
Na fila, o ponteiro de inserção está no fim e o de remoção (atendimento) está no início. Escrever e ler do CSV sem perder o sincronismo de qual seria a próxima senha a ser chamada e garantir que os metadados das senhas já atendidas não corrompessem a fila ativa exigiu lógica rigorosa de controle de ponteiros (head e tail) durante o parseamento do arquivo.

O Obstáculo Comum de Sintaxe (Parsing com sscanf):
Para os três programas, isolar os campos delimitados por vírgula ou ponto e vírgula sem deixar que quebras de linha (\n) ou espaços em branco corrompessem as strings (como nomes de músicas ou URLs) exigiu o abandono do scanf convencional e o uso de máscaras de leitura avançadas (como %99[^\n]), além de rotinas manuais de limpeza de buffer.

3. Conclusão
Embora os trabalhos tenham sido executados de forma independente e paralela, a experiência de desenvolvimento demonstrou que a escolha da estrutura de dados ideal (Lista, Pilha ou Fila) resolve o problema de organização em memória, mas a persistência de dados (estado em disco) exige um esforço lógico à parte. Cada modelo linear demandou uma estratégia de leitura e escrita customizada para que as propriedades LIFO, FIFO e sequenciais fossem mantidas intactas após o fechamento do console.
