#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


typedef struct client{
    char* name;
    int id;
} Client;

typedef struct data{
    //Client* client;
    //char** buffer;
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
    char x = 'x';
    char* reg = "register";
    char* login = "login";

    while(buffer[0] != x){
        if (client->id < 0)
        {
            perror("ERROR on accept");
            exit(3);
        }

        bzero(buffer,256);
        n = read(client->id, buffer, 255);
        if (n < 0)
        {
            perror("Error reading from socket");
            exit(4);
        }
        printf("Here is the message: %s\n", buffer);

        char* input = buffer;
        char delimiter[] = " ";
        char *firstWord, *context;

        int inputLength = strlen(input);
        char *inputCopy = (char*) calloc(inputLength + 1, sizeof(char));
        strncpy(inputCopy, input, inputLength);

        firstWord = strtok_r (inputCopy, delimiter, &context);
        printf("%s\n", firstWord);
        free(inputCopy);

        char* msg = "Random sprava";
        if(strcmp(reg, firstWord) == 0) {
            msg = "Uspesne si sa registroval!";
        }
        if(strcmp(login, firstWord) == 0) {
            msg = "Uspesne si sa prihlasil!";
        }

        n = write(client->id, msg, strlen(msg)+1);
        if (n < 0)
        {
            perror("Error writing to socket");
            exit(5);
        }
    }
    printf("Client(%d) ukoncil spojenie!\n", client->id);
    close(client->id);
}

void *print(void *d){
    Data* data = d;
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(25565);

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

    Data data = {0, 10, 100, &mutex, &cGenerate, &cPrint};

    pthread_create(&printer, NULL, print, &data);

    pthread_join(printer, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cGenerate);
    pthread_cond_destroy(&cPrint);

    return 0;
}