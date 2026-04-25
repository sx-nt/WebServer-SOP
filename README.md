# Instruções de Compilação

## Compilação normal
```
gcc sensor.c -o sensor -lpthread -lrt
gcc webserver.c -o webserver -lpthread -lrt

```

## Compilação com teste de contenção do mutex
```
gcc sensor.c -o sensor -lpthread -lrt -DTESTE_MUTEX
gcc webserver.c -o webserver -lpthread -lrt -DTESTE_MUTEX
```

--- 

# Execução 
## Terminal 1
```
./sensor
```


## Terminal 2
```
./webserver
```

Acesse 

```
http://localhost:8080 ou curl http://localhost:8080
```
--- 

##  Uso da Memória Compartilhada
A memória compartilhada POSIX permite que os processos `sensor` e `webserver` acessem a mesma região de memória.

O sensor grava a leitura mais recente.
O servidor lê essa leitura para responder aos clientes.
Isso evita uso de arquivos, pipes ou sockets para comunicação interna.
A troca de dados ocorre de forma rápida e eficiente.

Funções utilizadas:

* `shm_open()` — cria/abre a região compartilhada.
* `ftruncate()` — define seu tamanho.
* `mmap()` — mapeia a região no espaço de endereçamento do processo.

--- 

## Uso do Mutex Compartilhado entre Processos

O mutex fica dentro da própria memória compartilhada.
Ele é configurado com:

```
pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
```

Assim, pode ser utilizado por processos distintos.

Sua função é proteger a região crítica onde ocorre:

* escrita do sensor;
* leitura do servidor.

Isso evita condições de corrida e garante consistência dos dados.