#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

//variables
int socketSubs;
struct sockaddr_in address;
int puerto;
char *ip;
char *topico;
char buffer[1024] = {0};
char *bufferSplit[2];
int error = 0;
//pub
char topicoPub[1024];
char *pubSplit[20];
//sub
char *subSplit[20];
 

//bufferSeparator
void bufferSeparator(char *buffer, char *bufferSplit[], char *sep) {
  	char *mensaje;
  	int i = 0;
  	mensaje = strtok(buffer, sep);

  	while(mensaje != NULL) {
    		bufferSplit[i] = mensaje;
    		mensaje = strtok(NULL, sep);
    		i++;
  	}
}

void analisis(char *bufferSplit[]) {

  //Setea NULL todo el contenido
  memset(pubSplit, 0, sizeof(pubSplit));
  bufferSeparator(bufferSplit[0], pubSplit, "/");
  int i = 0;
  while(pubSplit[i] != NULL && subSplit[i] != NULL) {
    if(strcmp(subSplit[i], "+") == 0){
      i++;
      continue;
    }
    else if(strcmp(subSplit[i], pubSplit[i]) == 0 && subSplit[i+1] == NULL && pubSplit[i+1] == NULL) {
      printf("Nuevo mensaje recibido con TOPICO %s: %s\n", topicoPub, bufferSplit[1]);
      break;
    }
    else if(strcmp(subSplit[i], pubSplit[i]) != 0) {
      break;
    }
   i++;
  }
}



//proceso socket
void socket_subs(){
	pid_t pid;
	pid = fork();
	if(pid == 0){
		socketSubs = socket(AF_INET, SOCK_STREAM, 0);
  		if(socketSubs < 0) {
			error = 1;
    			perror("Error creando socket para subscriber");
    			exit(1);
  		}
  		printf("SUBSCRIBER ENCENDIDO\n");

  		address.sin_family = AF_INET;
  		address.sin_port = htons(puerto);
		address.sin_addr.s_addr = inet_addr(ip);

  		if(connect(socketSubs, (struct sockaddr *)&address, sizeof(address)) < 0){
    			error = 1;
			perror("Error de conexion");
    			close(socketSubs);
    			exit(1);
  		}
  		printf("Enviando topico al broker...\n");

  		if(write(socketSubs, topico, strlen(topico)) < 0) {
			error = 1;
    			perror("Error al enviar topico al Broker");
    			close(socketSubs);
    			exit(1);
  		}
		printf("Subscriber suscrito al BROKER con el tópico: %s\n", topico);

	}else{
		wait(NULL);
		if(error = 1){
			exit(1);
		}
	}
}

int main(int argc, char **argv) {

	if(argc != 7) {
    		perror("Formato: ./subscriber -i direcciónIP -p puerto -t topico");
    		return -1;
  	}

  	char opt;

  	while((opt = getopt(argc, argv, "i:p:t:")) != -1) {
    		switch(opt) {
      			case 'i':
        		 ip = optarg;
        		 break;
      			case 'p':
        		 puerto = atoi(optarg);
        		 break;
      			case 't':
        		 topico = optarg;
        		 break;
      			default:
        		 break;
    		}
  	}
  	//Proceso para creacion de socket y envio al broker
  	socket_subs();

	//sub multinivel
  	memset(subSplit, 0, sizeof(subSplit));
  	bufferSeparator(topico, subSplit, "/");


  	//recibir del broker
  	while(1) {
    		if(read(socketSubs, buffer, 1024) < 0) {
                	perror("Error read");
                	close(socketSubs);
                	exit(1);
    		}
    		bufferSeparator(buffer, bufferSplit, "$");
		strcpy(topicoPub, bufferSplit[0]);
		analisis(bufferSplit); //wildcards

    		memset(buffer, 0, sizeof(buffer));
  	}

  	close(socketSubs);
  	return 0;
}
