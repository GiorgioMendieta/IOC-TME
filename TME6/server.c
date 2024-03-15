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


void error(const char *msg)
{
        perror(msg);
        exit(1);
}

char *get_nom_from_buffer(char *str){
        char *res = malloc(sizeof(str));
        strcpy(res, str);
        char *res_to_return = res;
        while(*res != ' ' && res != '\0')
                res++;
        *res = '\0';
        return res_to_return;
}

char *get_vote_from_buffer(char *str){
        char *res = malloc(sizeof(str));
        strcpy(res, str);
        while(*res != ' ' && res != '\0')
                res++;
        return ++res;
}


int main(int argc, char *argv[])
{
        int sockfd, newsockfd, portno, DBfd;
        socklen_t clilen;
        char buffer[256];
        struct sockaddr_in serv_addr, cli_addr;
        int n;

        if (argc < 2) {
                fprintf(stderr, "ERROR, no port provided\n");
                exit(1);
        }

        // Ouverture de la database
        DBfd = open("./DataBase.txt", O_RDWR);
        if(DBfd < 0)
                error("ERROR opening database");
        // test
        // strcpy(buffer,"bonjour\n");
        // if(write(DBfd, buffer, 8)<0)
        //         error("ERROR écriture");
        


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
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR on binding");


        // On commence à écouter sur la socket. Le 5 est le nombre max
        // de connexions pendantes

        listen(sockfd, 5);
        while (1) {
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0)
                    error("ERROR on accept");

                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0)
                    error("ERROR reading from socket");

                // printf("Received packet from %s:%d\nData: [%s]\n\n",
                //        inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port),
                //        buffer);
                //sprintf(buffer,"%s\n", );
                printf("%s a voté %s\n", get_nom_from_buffer(buffer), get_vote_from_buffer(buffer));
                strcat(buffer, "\n");
                if(write(DBfd, buffer, sizeof(buffer))<0)
                        error("ERROR écriture");

                close(newsockfd);
        }

        close(sockfd);
        return 0;
}