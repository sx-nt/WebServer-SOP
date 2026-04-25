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

    recv(cliente, buffer, sizeof(buffer) - 1, 0);

    unsigned long tid = (unsigned long)pthread_self();

    int r = pthread_mutex_trylock(&shm->mutex);

    if (r == 0) {
        printf("[Thread %lu] Mutex obtido imediatamente.\n", tid);
    } else {
        printf("[Thread %lu] Mutex ocupado. Aguardando...\n", tid);
        pthread_mutex_lock(&shm->mutex);
        printf("[Thread %lu] Mutex obtido apos espera.\n", tid);
    }

#ifdef TESTE_MUTEX
    usleep(300000);
#endif

    if (shm->inicializado) {
        copia = shm->dado;
        tem_dado = 1;
        printf("[Thread %lu] Leitura copiada: #%d %.2f %s\n",
               tid, copia.contador, copia.temperatura, copia.status);
    }

    pthread_mutex_unlock(&shm->mutex);
    printf("[Thread %lu] Mutex liberado.\n", tid);

    if (tem_dado) {
        snprintf(resposta, sizeof(resposta),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n\r\n"
                 "<html><body>"
                 "<h2>Temperatura atual: %.2f C</h2>"
                 "<p>Leitura: %d</p>"
                 "<p>Status: %s</p>"
                 "</body></html>",
                 copia.temperatura,
                 copia.contador,
                 copia.status);
    } else {
        snprintf(resposta, sizeof(resposta),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n\r\n"
                 "<html><body>"
                 "<h2>Sensor ainda nao enviou nenhuma leitura.</h2>"
                 "<p>Aguarde alguns segundos e atualize a pagina.</p>"
}