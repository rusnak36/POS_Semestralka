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
    int newsockfd;
} Client;

typedef struct data{
    int index;
    Client* client;
    int size;
    pthread_mutex_t* mutex;
    pthread_cond_t* cGenerate;
    pthread_cond_t* cPrint;
} Data;

void *generate(void *d){

    Data* data = d;

    pthread_mutex_lock(data->mutex);
    Client* client = data->client+data->index;
    client->newsockfd = data->index;
    pthread_mutex_unlock(data->mutex);


    int n;
    char buffer[256];

    while(1){

        if (client->newsockfd < 0) {
            perror("ERROR on accept");
            exit(3);
        }

        bzero(buffer,256);
        n = read(client->newsockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            exit(4);
        }
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
        char* user;
        char text[201];

        if(!strcmp(buffer, "quit\n")){
            printf("rovnaju sa\n");
            command = "quit";
        }else{
            printf("nerovnaju sa\n");
            command = strtok(buffer, " ");
        }

        // JEDNOTLIVE SPRAVY
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

//            char tmp[256];
//            strcat(tmp, name);
//            strcat(tmp, " ");
//            strcat(tmp, password);
//            strcat(tmp, "\n");
//            printf("tmp: %s\n", tmp);

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
            }

            fclose(fptr);

            //logniho alebo ho posli dopice

            if(jeVsubore) {
                n = write(client->newsockfd, "ok", 3);
                client->name = strdup(name);
            }else {
                n = write(client->newsockfd, "nok", 4);
            }

            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }

            continue;

        } else if(!strcmp(command, "reg")) {
            printf("snazim sa registrovat uzivatela\n");
            //checkni txt ci tam neni meno obsadene
            //registrujho alebo ho posli dopice
            name = strtok(NULL, " ");
            password = strtok(NULL, " ");
            printf("snazim sa registrovat uzivatela!!!!!!!!!!!!!\n");
            //checkni txt ci tam existuje
            FILE *fptr;
            fptr = fopen("/home/hubocan9/userData.txt","r");
            if(fptr == NULL){
                printf("Error! neviem otvorit subor.\n");
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
                    jeVsubore = true;
                    break;
                }
            }
            fclose(fptr);

            //registruj ho alebo ho posli dopice

            if(jeVsubore) {
                n = write(client->newsockfd, "Meno je obsadene.", 18); //mozno ojeb o jednotku
            }else {
                fptr = fopen("/home/hubocan9/userData.txt","a");
                fprintf(fptr, tmp);
                fclose(fptr);
                n = write(client->newsockfd, "Boli ste uspesne registrovany.", 32); //mozno ojeb o jednotku
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
                n = write(client->newsockfd, "Teraz vas prihlasime.", 22);//mozno ojeb o jednotku
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
                client->name = strdup(name);
            }
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
            continue;

        } else if (!strcmp(command, "msg")) {
            printf("ZACINA MSG BS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            user = strtok(NULL, " ");
            printf("user: %d\n", user);
            char * token = strtok(NULL, " ");
            printf("token: %s\n", token);
            bzero(text,201);

            while(token != NULL){
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");

                printf("token: %s\n", token);
            }
            text[strlen(text)-1] = 0;


            printf("Client(%d)\n", client->newsockfd);
            printf("Pouzil prikaz: %s\n", command);
            printf("Pre osobu: %s\n", user);
            printf("S obsahom: %s\n", text);

            FILE *fptr;
            fptr = fopen("/home/hubocan9/msgLog.txt","a");
            if(fptr == NULL){
                printf("Error! neviem otvorit subor.\n");
                break;
            }
            char *fetak[300];
            bzero(fetak,300);
            strcat(fetak, "n ");
            strcat(fetak, client->name);
            strcat(fetak, " ");
            strcat(fetak, name);
            strcat(fetak, " ");
            strcat(fetak, text);
            printf("line2: %s",fetak);
            fprintf(fptr, fetak);
            fclose(fptr);

            char tmp [256];
            bzero(tmp,256);
            strcat(tmp, client->name);
            strcat(tmp, ": ");
            strcat(tmp, text);

            printf("pozliepany string co posiela server clientovy: %s\n",tmp);


            // x je cislo na ktory socket treba poslat spravu
            // na zaciatku je nastaveny samemu sebe
            int x = client->newsockfd;
            for (int i = 4; i < data->size; i++) {
                if(!strcmp(data->client[i].name, user)) {
                    x = data->client[i].newsockfd;
                    break;
                }
            }
            n = write(x, tmp, strlen(tmp));// +1 za
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
        } else if(!strcmp(command, "quit")){
            printf("idem quitnut usera: %s\n", client->name);

            n = write(client->newsockfd, "terminujem ta", 14);
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
            client->name = "";
            close(client->newsockfd);
            client->newsockfd = 0;
            printf("Bastard user died!\n");
            break;
        }
    }
}

void *print(void *d){
    Data* data = d;
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int portt = 26100;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portt);// ************************************ PREPISOVACIA MARHA ***********************************************************************************

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        portt++;
        serv_addr.sin_port = htons(portt);
        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Error binding socket address");
            exit(2);
        }
    }
    printf("port: %d\n",portt);


    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);


    int pocetClientov = 1;
    pthread_t generator;
    while(data->index < data->size){

        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        pocetClientov++;

        data->index=newsockfd;

        pthread_create(&generator, NULL, generate, data);
    }
    for(int i = 0; i < pocetClientov; i++) {
        pthread_join(generator, NULL);
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

    Client client[100];

    for (int i = 0; i < 100; ++i) {
        client[i].name = "";
    }

    Data data;
    data.index=0;
    data.size=100;
    data.client = client;
    data.mutex = &mutex;



    pthread_create(&printer, NULL, print, &data);

    pthread_join(printer, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cGenerate);
    pthread_cond_destroy(&cPrint);

    return 0;
}