#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "dados.h"

#define PORTA 8080
#define BUFFER_SIZE 4096

shm_dados_t *shm;

void *atender_cliente(void *arg) {
    int cliente = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    char resposta[BUFFER_SIZE];
    dado_t copia;
    int tem_dado = 0;

    // Recebe a requisição do navegador (embora não processemos o conteúdo aqui)
    recv(cliente, buffer, sizeof(buffer) - 1, 0);

    unsigned long tid = (unsigned long)pthread_self();

    // Tenta obter o mutex para ler os dados
    int r = pthread_mutex_trylock(&shm->mutex);

    if (r == 0) {
        printf("[Thread %lu] Mutex obtido imediatamente.\n", tid);
    } else {
        printf("[Thread %lu] Mutex ocupado. Aguardando...\n", tid);
        pthread_mutex_lock(&shm->mutex);
        printf("[Thread %lu] Mutex obtido apos espera.\n", tid);
    }

#ifdef TESTE_MUTEX
    usleep(300000); // Atraso artificial para teste
#endif

    // Copia os dados da memória compartilhada para uma variável local
    if (shm->inicializado) {
        copia = shm->dado;
        tem_dado = 1;
        printf("[Thread %lu] Leitura copiada: #%d %.2f %s\n",
               tid, copia.contador, copia.temperatura, copia.status);
    }

    pthread_mutex_unlock(&shm->mutex);
    printf("[Thread %lu] Mutex liberado.\n", tid);

    // Monta a resposta HTTP
    if (tem_dado) {
        snprintf(resposta, sizeof(resposta),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n\r\n"
                 "<html><body>"
                 "<h2>Temperatura atual: %.2f C</h2>"
                 "<p>Leitura: %d</p>"
                 "<p>Status: %s</p>"
                 "</body></html>",
                 copia.temperatura, copia.contador, copia.status);
    } else {
        snprintf(resposta, sizeof(resposta),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n\r\n"
                 "<html><body>"
                 "<h2>Sensor ainda nao enviou nenhuma leitura.</h2>"
                 "<p>Aguarde alguns segundos e atualize a pagina.</p>"
                 "</body></html>");
    }

    // Envia para o navegador e fecha a conexão
    send(cliente, resposta, strlen(resposta), 0);
    close(cliente);
    return NULL;
}

int main() {
    // Abre a memória compartilhada
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open (servidor precisa que o sensor rode primeiro)");
        exit(EXIT_FAILURE);
    }

    shm = mmap(NULL, sizeof(shm_dados_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Configuração do Socket
    int servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;
    endereco.sin_port = htons(PORTA);

    int opt = 1;
    setsockopt(servidor_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(servidor_fd, (struct sockaddr *)&endereco, sizeof(endereco)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(servidor_fd, 10);
    printf("Servidor Web rodando em http://localhost:%d\n", PORTA);

    while (1) {
        struct sockaddr_in cliente_addr;
        socklen_t addr_len = sizeof(cliente_addr);
        int *novo_cliente = malloc(sizeof(int));
        *novo_cliente = accept(servidor_fd, (struct sockaddr *)&cliente_addr, &addr_len);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, atender_cliente, novo_cliente);
        pthread_detach(thread_id); // A thread se limpa sozinha ao terminar
    }

    return 0;
}