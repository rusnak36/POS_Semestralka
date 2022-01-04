#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

void *mWrite(int sockfd){
    int n;
    char buffer[256];

    while(1){
        bzero(buffer,256);
        fgets(buffer, 255, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
    }
    return NULL;
}

void *mRead(int sockfd){
    int n;
    char buffer[256];

    while(1){
        bzero(buffer,256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
        {
            perror("Error reading from socket");
            return 6;
        }
        printf("%s\n",buffer);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    // register/login
    char registered[2];
    char name[11];
    char password[11];
    bool log = false;


    while(1){
        printf("Are you a registered user?? (y/n)\n");
        scanf("%s", registered);

        if(strcmp(registered, "y") == 0 || strcmp(registered, "Y") == 0){
            //login
            printf("is registered\n");
            printf("To login please put your nickname and password.\n");

            printf("Name: ");
            scanf("%s", name);

            printf("Password: ");
            scanf("%s", password);

            //
            strcat(buffer, "log ");
            strcat(buffer, name);
            strcat(buffer, " ");
            strcat(buffer, password);
            log = true;

            n = write(sockfd, buffer, strlen(buffer+1));
            if (n < 0) {
                perror("Error writing to socket");
                return 6;
            }
            break;
        }
        if(strcmp(registered, "n") == 0 || strcmp(registered, "N") == 0){
            //register
            printf("is not registered\n");
            printf("To register please put your desired nickname and password.\n");

            printf("Name: ");
            scanf("%s", name);

            printf("Password: ");
            scanf("%s", password);

            //poslem serveru ziadost o registraciu s udajmi name password
            //vypisem ci sa podarila
            strcat(buffer, "log ");
            strcat(buffer, name);
            strcat(buffer, " ");
            strcat(buffer, password);

            n = write(sockfd, buffer, strlen(buffer+1));
            if (n < 0) {
                perror("Error writing to socket");
                return 6;
            }
            break;
        }
        printf("bad input. try again\n");
    }

    scanf("%c", (char *) stdin);
    printf("\npost log/reg\n");

    //ak si sa prihlasoval
    if(log){
        if(1){ // spravne udaje
            pthread_t tRead;
            pthread_t tWrite;

            pthread_create(&tRead, NULL, &mRead, sockfd);
            pthread_create(&tWrite, NULL, &mWrite, sockfd);

            pthread_join(tRead, NULL);
            pthread_join(tWrite, NULL);
        }else{ // nespravne udaje
            printf("Better learn how to type mate!");
        }


    }else{ //registroval si sa
        if(1){ // registracne udaje vyhovuju
            memset(buffer,0,strlen(buffer));
            // pokus o login
            strcat(buffer, "log ");
            strcat(buffer, name);
            strcat(buffer, " ");
            strcat(buffer, password);

            n = write(sockfd, buffer, strlen(buffer+1));
            if (n < 0) {
                perror("Error writing to socket");
                return 6;
            }

        }else{ // registracne udaje nevyhovuju
            printf("You need a new name mate!\n");
        }
    }

    close(sockfd);

    return 0;
}