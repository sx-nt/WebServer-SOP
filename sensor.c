#include <stdio.h>

    shm = mmap(NULL, sizeof(shm_dados_t),
               PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if (!shm->inicializado) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shm->mutex, &attr);
        pthread_mutexattr_destroy(&attr);

        shm->inicializado = 0;
        shm->dado.contador = 0;
    }

    srand(time(NULL));

    while (1) {
        dado_t novo;

        novo.temperatura = 20.0f + (rand() % 2000) / 100.0f;
        novo.contador = shm->dado.contador + 1;

        if (novo.temperatura > 30.0f)
            strcpy(novo.status, "ALERTA");
        else
            strcpy(novo.status, "NORMAL");

        printf("\n[Sensor] Nova leitura #%d: %.2f C - %s\n",
               novo.contador, novo.temperatura, novo.status);

        int r = pthread_mutex_trylock(&shm->mutex);

        if (r == 0) {
            printf("[Sensor] Mutex obtido imediatamente.\n");
        } else {
            printf("[Sensor] Mutex ocupado. Aguardando...\n");
            pthread_mutex_lock(&shm->mutex);
            printf("[Sensor] Mutex obtido apos espera.\n");
        }

#ifdef TESTE_MUTEX
        usleep(300000);
#endif

        shm->dado = novo;
        shm->inicializado = 1;

        printf("[Sensor] Dados gravados na memoria compartilhada.\n");

        pthread_mutex_unlock(&shm->mutex);
        printf("[Sensor] Mutex liberado.\n");

        sleep(1);
    }

    munmap(shm, sizeof(shm_dados_t));
    close(fd);
    return 0;
}