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
        printf("\n");
        printf("vypis obsahu buffera z write thready: %s",buffer);
        printf("pocet znakov(aj s koncovym bielim znakom): %d\n",(int)strlen(buffer));

        char *dup = strdup(buffer);
        char* command;
        command = strtok(dup, " ");
        if(!strcmp(command, "msgC")){
            printf("je to msgC\n");
            char* komu;
            char text[201];

            komu = strtok(NULL, " ");
            printf("komu: %s\n",komu);

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
            printf("text: %s\n", text);
            printf("text dlzka(aj s novym riadkom nakonci): %d\n", (int)strlen(text));

            //cezar
            for(int i=0; i < strlen(text)-1; i++) {
                text[i] += 1;
            }
            //shift podla pozicie
            int shift = 0;
            for(int i=0; i < strlen(text)-1; i++) {
                shift = i % 4;
                text[i] += shift;
            }
            //vymen znaky vedla seba

            if((strlen(text)-1) % 2 == 0){
                for(int i=0; i<strlen(text)-1; i+=2){
                    char c = text[i];
                    text[i] = text[i+1];
                    text[i+1] = c;
                }
            }else{
                for(int i=0; i<strlen(text)-2; i+=2){
                    char c = text[i];
                    text[i] = text[i+1];
                    text[i+1] = c;
                }
            }


            char* crypted[300];
            bzero(crypted,300);
            strcat(crypted, command);
            strcat(crypted, " ");
            strcat(crypted, komu);
            strcat(crypted, " ");
            strcat(crypted, text);
            printf("original: %s", buffer);
            printf("    copy: %s\n", crypted);

            n = write(sockfd, crypted, strlen(crypted)+1);
            if (n < 0) {
                perror("Error writing to socket");
                return 5;
            }
        }else{
            n = write(sockfd, buffer, strlen(buffer)+1);
            if (n < 0) {
                perror("Error writing to socket");
                return 5;
            }
            if(!strcmp(buffer, "quit\n")){
                break;
            }

        }
        free(dup);
    }
}

void *mRead(int sockfd){
    int n;
    char buffer[256];

    while(1){
        bzero(buffer,256);
        n = read(sockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return 6;
        }

        if(!strcmp(buffer, "terminujem ta")){
            printf("reeeeeeeeeeeeeeeee\n");
            break;
        }

        printf("sprava ktora prisla: %s\n",buffer);

        char* command;
        command = strtok(buffer, " ");
        printf("command: %s\n", command);
        if(!strcmp(command, "s")){
            printf("dekodovanie: \n");

            char* odkial;
            char text[201];
            bzero(text,201);
            odkial = strtok(NULL, " ");
            printf("odkial %s\n", odkial);

            char * token = strtok(NULL, " ");
            printf("token: %s\n",token);

            while(token != NULL){
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");
            }
            text[strlen(text)-1] = 0;

            if((strlen(text)-1) % 2 == 0){
                for(int i=0; i<strlen(text)-1; i+=2){
                    char c = text[i];
                    text[i] = text[i+1];
                    text[i+1] = c;
                }
            }else{
                for(int i=0; i<strlen(text)-2; i+=2){
                    char c = text[i];
                    text[i] = text[i+1];
                    text[i+1] = c;
                }
            }
            int shift = 0;
            for(int i=0; i < strlen(text)-1; i++) {
                shift = i % 4;
                text[i] -= shift;
            }
            for(int i=0; i < strlen(text)-1; i++){
                text[i] -= 1;
            }

            char decrypted[300];
            bzero(decrypted,300);
//            strcat(decrypted, command);

//            strcat(decrypted, " ");
            strcat(decrypted, odkial);
            strcat(decrypted, " ");
            printf("text: %s\n",text);
            strcat(decrypted, text);
            printf("%s\n", decrypted);

        }else if (!strcmp(command, "n")){
            printf("nesifrovana sprava:\n");

            char* odkial;
            char text[201];
            odkial = strtok(NULL, " ");
            char * token = strtok(NULL, " ");
            bzero(text,201);


            while(token != NULL){
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");
            }
            text[strlen(text)-1] = 0;
            char final[300];
            bzero(final, 300);
            strcat(final,odkial);
            strcat(final," ");
            strcat(final,text);
            printf("%s\n", final);
        }else if(!strcmp(command, "show")){
            printf("I AM HERE BITCHES!\n");

            char text[300];
            char * token = strtok(NULL, " ");
            bzero(text,300);
            while(token != NULL){
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");
            }
            text[strlen(text)-1] = 0;
            printf("online su: %s\n", text);
        }
        else if(!strcmp(command, "friendRequest")){
            char* newfriend;
            newfriend = strtok(NULL, " ");
            printf("%s si ta chce pridat ako priatela!\n",newfriend);
        } else if(!strcmp(command, "noFriendsForYou")){
            printf("%s tvoj friend request odmietol!\n", strtok(NULL, " "));

        }

    }
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

    int portt = 26100;
    serv_addr.sin_port = htons(portt);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        portt++;
        serv_addr.sin_port = htons(portt);
        if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Error connecting to socket");
            return 4;
        }
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

            printf("obsah buffera odoslaneho na server: %s\n", buffer);
            printf("dlzka buffera(bez +1): %d\n", (int)strlen(buffer));

            n = write(sockfd, buffer, strlen(buffer));
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
            strcat(buffer, "reg ");
            strcat(buffer, name);
            strcat(buffer, " ");
            strcat(buffer, password);
            printf("NA SERVER POSIELAM TENTO STRING: %s\n", buffer);

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                perror("Error writing to socket");
                return 6;
            }
            break;
        }
        printf("bad input. try again\n");
    }

    printf("\npost log/reg\n");

    //ak si sa prihlasoval
    if(log){
        n = read(sockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return 6;
        }
        printf("obsah buffera odoslaneho z serveru: %s\n", buffer);
        printf("dlzka buffera: %d\n", (int)strlen(buffer));

        if(!strcmp(buffer, "ok")){ // spravne udaje
            scanf("%c", (char *) stdin);
            sleep(1);
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
        n = read(sockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return 6;
        }
        printf("obsah buffera odoslaneho z serveru: %s\n", buffer);
        int result = strcmp(buffer, "Boli ste uspesne registrovany.");
        printf("result: %d\n",result);

        if(!strcmp(buffer, "Boli ste uspesne registrovany.")){ // spravne udaje

            pthread_t tRead;
            pthread_t tWrite;

            pthread_create(&tRead, NULL, &mRead, sockfd);
            pthread_create(&tWrite, NULL, &mWrite, sockfd);

            pthread_join(tRead, NULL);
            pthread_join(tWrite, NULL);
        }else{ // nespravne udaje
            printf("boha jeho treba novy nick!");
        }
    }
    close(sockfd);
    return 0;
}