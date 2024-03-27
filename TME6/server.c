/* A simple server in the internet domain using TCP The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "hashTab.h"

// Print error message and exit
void error(const char *msg)
{
        // Display the error message on the standard error output
        perror(msg);
        exit(1);
}

// Get the voter's name from the buffer
char *get_nom_from_buffer(char *str){
        char *res = malloc(sizeof(str));
        strcpy(res, str);
        char *res_to_return = res;

        // Iterate until the first space
        while(*res != ' ' && res != '\0')
                res++;
        *res = '\0';

        return res_to_return;
}

// Get the vote from the buffer
char *get_vote_from_buffer(char *str){
        char *res = malloc(sizeof(str));
        strcpy(res, str);
        
        // Iterate until the first space
        while(*res != ' ' && res != '\0')
                res++;
        return ++res;
}


int main(int argc, char *argv[])
{
        // Variables for the server
        int sockfd, newsockfd, portno, DBfd;
        socklen_t clilen;
        char buffer[256];
        struct sockaddr_in serv_addr, cli_addr;
        int n;

        // Verify that the port number is provided
        if (argc < 2) {
                fprintf(stderr, "ERROR, no port provided\n");
                fprintf(stderr, "Usage : ./server portnumber\n");
                exit(1);
        }

        // Ouverture de la database en lecture/écriture
        DBfd = open("./DataBase.txt", O_RDWR);
        if(DBfd < 0)
                error("ERROR opening database");
        // test
        // strcpy(buffer,"bonjour\n");
        // if(write(DBfd, buffer, 8)<0)
        //         error("ERROR écriture");
        
        // Création de la structure hashTab
        struct hashTab *hashtab = init(100);


        // 1) on crée la socket, SOCK_STREAM signifie TCP

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");

        // 2) on réclame au noyau l'utilisation du port passé en paramètre 
        // INADDR_ANY dit que la socket va être affectée à toutes les interfaces locales

        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = atoi(argv[1]);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        // Bind the socket to the port number 
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR on binding");


        // On commence à écouter sur la socket. Le 5 est le nombre max
        // de connexions pendantes

        listen(sockfd, 5);
        while (1) {
                // Wait for a connection
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                // Check if the connection was successful
                if (newsockfd < 0)
                    error("ERROR on accept");
                
                // Fill the buffer with zeros
                bzero(buffer, 256);
                // Read the message from the client and write it to the buffer
                n = read(newsockfd, buffer, 255);
                // Check if the read was successful
                if (n < 0)
                    error("ERROR reading from socket");
                
                // Get the voter's name and vote from the buffer
                char *nom = get_nom_from_buffer(buffer);
                char *vote = get_vote_from_buffer(buffer);

                // printf("Received packet from %s:%d\nData: [%s]\n\n",
                //        inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
                //        buffer);
                // sprintf(buffer,"%s\n", );
                
                /*
                printf("%s a voté %s\n", get_nom_from_buffer(buffer), get_vote_from_buffer(buffer));
                strcat(buffer, "\n"); // ajoute un retour à la ligne
                if(write(DBfd, buffer, sizeof(buffer))<0) // on écrit dans la database
                        error("ERROR écriture");
                */

                if(!existe(hashtab, nom)){
                        insere(hashtab, nom, vote);
                        printf("%s a voté %s\n", nom, vote);
                } else {
                        printf("%s a déjà voté %s\n", nom, vote);
                }

                close(newsockfd);
        }

        close(sockfd);
        return 0;
}