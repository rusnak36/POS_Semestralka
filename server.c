#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>


typedef struct client{
    char* name;
    int id;
} Client;

typedef struct data{
    Client* client;
    int index;
    int k;
    int n;
    pthread_mutex_t* mutex;
    pthread_cond_t* cGenerate;
    pthread_cond_t* cPrint;
} Data;

void *generate(void *d){
    Client* client = d;
    int n;
    char buffer[256];

    while(1){

        if (client->id < 0) {
            perror("ERROR on accept");
            exit(3);
        }

        bzero(buffer,256);
        n = read(client->id, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            exit(4);
        }
        printf(buffer);
        printf("Here is the message: %s\n", buffer);
        if(strlen(buffer) < 2) {
            printf("Preventol som kokotinu!!!\n");
            sleep(3);
            continue;
        }

        char* msg = buffer;
        char* command;
        char *name;
        char *password;
        int user;
        char text[201];

        command = strtok(buffer, " ");

        printf("Here is the message4: %s\n", buffer);
        printf("\n");

        if(!strcmp(command, "log")){
            name = strtok(NULL, " ");
            password = strtok(NULL, " ");
            printf("snazim sa lognut uzivatela!!!!!!!!!!!!!\n");
            //checkni txt ci tam existuje
            FILE *fptr;
            fptr = fopen("/home/hubocan9/userData.txt","r");
            if(fptr == NULL){
                printf("Error! neviem otvorit\n");
                break;
            }
            char line[256];
            bool jeVsubore = false;

            char tmp[256];
            strcat(tmp, name);
            strcat(tmp, " ");
            strcat(tmp, password);
            strcat(tmp, "\n");
            printf("tmp: %s\n", tmp);
            char* tname;
            char* tpassword;

            while(fgets(line, sizeof(line), fptr)){
                tname = strtok(line, " ");
                tpassword = strtok(NULL, " ");
                tpassword[strlen(tpassword)-1] = 0;

                if(!strcmp(tname, name)){
                    if(!strcmp(tpassword, password)){
                        jeVsubore = true;
                        break;
                    }
                }
                if(feof(fptr)){
                    printf("dosiel som na koniec suboru.\n");
                    break;
                }
            }

            fclose(fptr);

            //logniho alebo ho posli dopice
            //client->name = name;
            if(jeVsubore) {
                n = write(client->id, "ok", 3);
            }else {
                n = write(client->id, "nok", 4);
            }

            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }



        } else if(!strcmp(command, "reg")) {
            printf("snazim sa registrovat uzivatela\n");
            //checkni txt ci tam neni meno obsadene
            //registrujho alebo ho posli dopice

        } else if (!strcmp(command, "msg")) {
            printf("ZACINA MSG BS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            user = atoi(strtok(NULL, " "));
            printf("user: %d\n", user);
            char * token = strtok(NULL, " ");
            printf("token: %s\n", token);
            bzero(text,201);
            while(token != NULL){
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, "");
                printf("token: %s\n", token);
            }

            printf("Client(%d)\n", client->id);
            printf("Pouzil prikaz: %s\n", command);
            printf("Pre osobu: %d\n", user);
            printf("S obsahom: %s\n", text);

            char tmp [256];
            bzero(tmp,256);
            printf("tmp: %s\n",tmp);
            char tmpid[20];
            sprintf(tmpid, "client: %d", client->id);
            printf("tmpid: %s\n", tmpid);
            strcat(tmp, tmpid);
            strcat(tmp, ": ");
            strcat(tmp, text);

            printf("pozliepany string co posiela server clientovy: %s\n",tmp);
            n = write(user, tmp, strlen(tmp)+1);
            if (n < 0)
            {
                perror("Error writing to socket");
                exit(5);
            }
        } else if(!strcmp(command, "quit")){
            exit(69);
        }

    }
    close(client->id);
}

void *print(void *d){
    //Data* data = d;
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(26073);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket address");
        exit(2);
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    while(newsockfd > 0){
        pthread_t generator;
        Client* c = malloc(sizeof(Client));
        c->id = newsockfd;
        pthread_create(&generator, NULL, generate, c);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
    }

    close(sockfd);
}


int main(int argc, char *argv[])
{

    pthread_mutex_t mutex;
    pthread_cond_t cGenerate;
    pthread_cond_t cPrint;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cGenerate, NULL);
    pthread_cond_init(&cPrint, NULL);

    pthread_t printer;

    //Data data = {0, 10, 100, &mutex, &cGenerate, &cPrint};

    pthread_create(&printer, NULL, print, NULL);

    pthread_join(printer, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cGenerate);
    pthread_cond_destroy(&cPrint);

    return 0;
}