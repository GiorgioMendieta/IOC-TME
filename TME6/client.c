#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
        perror(msg);
        exit(0);
}

// Client takes 4 arguments: hostname, port, nomVotant, vote
int main(int argc, char *argv[])
{
        int sockfd, portno, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;

        char buffer[256];

        // Le client doit connaitre l'adresse IP du serveur, et son numero de port
        if (argc < 5) {
                fprintf(stderr,"Usage : ./client hostname port nomVotant vote(ete ou hiver)\n");
                exit(0);
        }

        // On récupère le numéro de port 
        portno = atoi(argv[2]);

        // 1) Création de la socket, INTERNET et TCP

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");

        server = gethostbyname(argv[1]);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
        }

        // On donne toutes les infos sur le serveur

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);

        // On se connecte. L'OS local nous trouve un numéro de port, grâce auquel le serveur
        // peut nous renvoyer des réponses, le \n permet de garantir que le message ne reste
        // pas en instance dans un buffer d'emission chez l'emetteur (ici c'est le clent).

        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR connecting");

        // Copy the voter name and vote into the buffer
        strcpy(buffer, argv[3]);
        strcat(buffer, " ");
        strcat(buffer, argv[4]);

        // On envoie le message au serveur
        n = write(sockfd, buffer, strlen(buffer));
        if (n != strlen(buffer))
                error("ERROR message not fully trasmitted");

        // On ferme la socket pour sortir proprement
        close(sockfd);
        return 0;
}