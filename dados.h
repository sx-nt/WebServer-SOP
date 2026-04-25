#ifndef DADOS_H
#define DADOS_H

#include <pthread.h>

#define SHM_NAME "/sensor_temperatura_shm"

typedef struct {
    float temperatura;
    int contador;
    char status[16];
} dado_t;

typedef struct {
    pthread_mutex_t mutex;
    int inicializado;
    dado_t dado;
} shm_dados_t;

#endif