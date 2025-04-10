#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

//variables
int socketsSubs[21];
int contadorSubs;
char *bufferSplit[2] = {};
char buffer[1024] = {0};
char bufferCopy[1024] = {0};
int cantidad = 0;
char registro[255]; // LOG

//mutex -n- semaforos
pthread_mutex_t mutex;
sem_t s_socket;
sem_t s_mensaje;

//log
void makeLog(char *registro) {
	pid_t pid;
  	pid = fork();
  	if(pid == 0) {
    		char comando[1024];
    		sprintf(comando, "logger --no-act -t Broker -s %s >> log.txt 2>&1", registro);
    		char *argumentos[] = {"sh","-c", comando, NULL};

    		execvp("/bin/sh", argumentos);
  	}
    	else {
    		wait(NULL);
  	}
}



//BufferSeparator
void bufferSeparator(char *buffer, char *bufferSplit[], char *sep) {
  	char *mensaje;
  	int i= 0;
  	mensaje = strtok(buffer, sep);
  	memset(bufferSplit,0,2*sizeof(bufferSplit[0]));
  	while(mensaje != NULL) {
    		bufferSplit[i] = mensaje;
    		mensaje = strtok(NULL, sep);
    		i++;
  	}
  	sem_post(&s_mensaje);
}

//hilo socket
void *hilo_socket(void *args){
	int new_socket = *(int *)args;
 	if(new_socket < 0) {
      		perror("Error de conexion con el socket del cliente");
      		close(new_socket);
      		exit(1);
    	}
        memset(buffer,0,sizeof(buffer));
  	if(read(new_socket, buffer, 1024) < 0) {
		if(strlen(buffer) == 0){
			close(new_socket);
		}
    		perror("Error read");
    		close(new_socket);
    		exit(1);
  	}
	printf("\n");
  	strcpy(bufferCopy,buffer);
  	bufferSeparator(bufferCopy, bufferSplit, "$");
	cantidad = 0; //here
  	sem_wait(&s_mensaje);
  	while(bufferSplit[cantidad] != NULL){
        	cantidad++;
  	}

	sem_post(&s_socket);
	return 0;
}

//hilo mqtt
void *hilo_mqtt(void *args) {
  	sem_wait(&s_socket);
  	int new_socket = *(int *)args;

  	if(cantidad == 1) {
    		sprintf(registro,"***NUEVO SUSCRIBER***");
        	printf("%s\n", registro);
        	makeLog(registro);// log
    		pthread_mutex_lock(&mutex);//mutex lock
    		socketsSubs[contadorSubs] = new_socket;
    		contadorSubs++;
    		pthread_mutex_unlock(&mutex);//mutex unlock
    		sprintf(registro,"Nuevo suscriber con tópico: %s", bufferSplit[0]);
        	printf("%s\n", registro);
        	makeLog(registro); //log
  	}else {
    		sprintf(registro,"***NUEVO PUBLISHER***");
        	printf("%s\n", registro);
        	makeLog(registro);//log
    		sprintf(registro,"Publisher envio al tópico: %s el Mensaje: %s", bufferSplit[0], bufferSplit[1]);
        	printf("%s\n", registro);
        	makeLog(registro);//log

    		for(int i = 0; i < contadorSubs; i++) {
      			if(write(socketsSubs[i], buffer, strlen(buffer)) < 0) {
        			sprintf(registro, "Error publisher al enviar mensaje");
				printf("%s\n", registro);
                		makeLog(registro); //log

      			}
    		}
  	}

  return 0;
}


int main(int argc, char **argv) {
  	if(argc != 5) {
    		perror("Formato: ./broker -i ip -p puerto");
    		return -1;
  	}

  	char opt;
  	int puerto;
  	char *ip;
  	int socketMqtt;
  	struct sockaddr_in address;

  	while((opt = getopt(argc, argv, "i:p:")) != -1) {
    		switch(opt) {
      			case 'i':
        		 ip = optarg;
        		 break;
      			case 'p':
        		 puerto = atoi(optarg);
        		 break;
      			default:
        		 break;
    		}
  	}
  	//inicializacion mutex -n- semaforos 
  	pthread_mutex_init(&mutex,NULL);
  	sem_init(&s_socket,0,0);
  	sem_init(&s_mensaje,0,0);

  	//Crear socket
  	socketMqtt = socket(AF_INET, SOCK_STREAM, 0);

  	if(socketMqtt < 0) {
    		perror("Error al crear el socket");
    		exit(1);
  	}

  	address.sin_family = AF_INET;
  	address.sin_addr.s_addr = inet_addr(ip);
  	address.sin_port = htons(puerto);

  	if(bind(socketMqtt, (struct sockaddr *)&address, sizeof(address)) < 0) {
    		perror("Error bind");
    		close(socketMqtt);
    		exit(1);
  	}
  
  	if(listen(socketMqtt, 128) < 0) {
    		perror("Error listen");
    		close(socketMqtt);
    		exit(1);
  	}
  	sprintf(registro,"BROKER ENCENDIDO");
	printf("%s\n", registro);
	makeLog(registro);
  	sprintf(registro,"...Escuchando...");
	printf("%s\n", registro);
	makeLog(registro);
  	while(1) {
    		int new_socket = accept(socketMqtt, NULL, NULL);
    		pthread_t hiloSocket;
    		pthread_t hiloMqtt;
    		pthread_create(&hiloSocket,NULL,hilo_socket, &new_socket);
    		pthread_create(&hiloMqtt, NULL, hilo_mqtt, &new_socket);
  	}

  return 0;

}
