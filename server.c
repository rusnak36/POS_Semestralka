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
    struct client* friends;
    struct client* request;
    bool nastaloMazanie;
} Client;

typedef struct data{
    int index;
    Client* client;
    int size;
    pthread_mutex_t* mutex;
    pthread_cond_t* cGenerate;
    pthread_cond_t* cPrint;
} Data;

void *listener(void*d){
    char input[256];
    Data *data = d;
    int n = 0;
    while(1){
        bzero(input,256);
        scanf("%s", input);
        if(!strcmp(input, "shutdown")){
            for(int i = 0; i < data->size; i++){
                n = write(data->client[i].newsockfd, "terminujem ta", 14);
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);

                }
            }
        }
    }
}

void *generate(void *d){


    Data* data = d;
    Client friends[50];

    pthread_mutex_lock(data->mutex);
    Client* client = data->client+data->index;
    client->newsockfd = data->index;
    client->friends = friends;
    client->request = NULL;
    pthread_mutex_unlock(data->mutex);

    for (int i = 0; i < 50; ++i) {
        client->friends[i].name = "*";
    }

    client->nastaloMazanie = false;
    int n;
    char buffer[256];

    while(1) {
        if(client->nastaloMazanie){
            Client autobus;
            autobus.name ="*";
            autobus.newsockfd = 0;
            autobus.friends = NULL;
            autobus.request = NULL;
            autobus.nastaloMazanie = false;
            client->friends[50] = autobus;
        }
        client->nastaloMazanie =false;

        bzero(buffer, 256);
        if (client->newsockfd < 0) {
            perror("ERROR on accept");
            exit(3);
        }

        bzero(buffer, 256);
        n = read(client->newsockfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            exit(4);
        }
        printf("Here is the message: %s\n", buffer);
        if (strlen(buffer) < 2) {
            printf("Preventol som kokotinu!!!\n");
            sleep(3);
            continue;

        }

        char *msg = buffer;
        char *command;
        char *name;
        char *password;
        char *user;
        char text[201];

        if (!strcmp(buffer, "quit\n")) {
            command = "quit";
        } else if (!strcmp(buffer, "show\n")) {
            command = "show";
        } else if(!strcmp(buffer, "decline\n")){
            command = "decline";
        } else if (!strcmp(buffer, "accept\n")) {
            command = "accept";
        } else if (!strcmp(buffer, "history\n")) {
            command = "history";
        } else {
            command = strtok(buffer, " ");
        }

        if (!strcmp(command, "log")) {
            name = strtok(NULL, " ");
            password = strtok(NULL, " ");
            FILE *fptr;
            fptr = fopen("/home/pos/userData.txt", "r");
            if (fptr == NULL) {
                printf("Error! neviem otvorit userData.txt\n");
                break;
            }
            char line[256];
            bool jeVsubore = false;

            char *tname;
            char *tpassword;

            while (fgets(line, sizeof(line), fptr)) {
                tname = strtok(line, " ");
                tpassword = strtok(NULL, " ");
                tpassword[strlen(tpassword) - 1] = 0;

                if (!strcmp(tname, name)) {
                    if (!strcmp(tpassword, password)) {
                        jeVsubore = true;
                        break;
                    }
                }
            }
            fclose(fptr);

            int fnum=0;
            char* tfriend;
            fptr = fopen("/home/pos/friendData.txt","r");
            if (!fptr) {
                printf("Neviem otvorit friendData.txt!!\n");
                return 0;
            }

            while (fgets(line, sizeof(line), fptr)) {
                tname = strtok(line, " ");
                if(tname == name){
                    fnum = atoi(strtok(NULL, " "));
                    for(int i=0; i < fnum; i++){
                        tfriend = strtok(NULL, " ");
                        client->friends[i].name = tfriend;
                    }
                }
            }
            fclose(fptr);

            //logniho alebo ho posli dopice

            if (jeVsubore) {
                n = write(client->newsockfd, "ok", 3);
                client->name = strdup(name);
            } else {
                n = write(client->newsockfd, "nok", 4);
            }
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }

            continue;

        } else if (!strcmp(command, "reg")) {
            name = strtok(NULL, " ");
            password = strtok(NULL, " ");
            FILE *fptr;
            fptr = fopen("/home/pos/userData.txt", "r");
            if (fptr == NULL) {
                printf("Error! neviem otvorit subor userData.txt.\n");
                break;
            }
            char line[256];
            bool jeVsubore = false;

            char tmp[256];
            strcat(tmp, name);
            strcat(tmp, " ");
            strcat(tmp, password);
            strcat(tmp, "\n");
            char *tname;
            char *tpassword;

            while (fgets(line, sizeof(line), fptr)) {
                tname = strtok(line, " ");
                tpassword = strtok(NULL, " ");
                tpassword[strlen(tpassword) - 1] = 0;

                if (!strcmp(tname, name)) {
                    jeVsubore = true;
                    break;
                }
            }
            fclose(fptr);

            if (jeVsubore) {
                n = write(client->newsockfd, "Meno je obsadene.", 18);
            } else {
                fptr = fopen("/home/pos/userData.txt", "a");
                fprintf(fptr, tmp);
                fclose(fptr);


                fptr = fopen("/home/pos/friendData.txt","a");
                fprintf(fptr, name);
                fprintf(fptr, " 0 \n");
                fclose(fptr);

                n = write(client->newsockfd, "Boli ste uspesne registrovany.", 31);
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
                n = write(client->newsockfd, "Teraz vas prihlasime.", 22);
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
            user = strtok(NULL, " ");
            char *token = strtok(NULL, " ");
            bzero(text, 201);
            bool jeOnline = false;

            while (token != NULL) {
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");
            }
            text[strlen(text) - 1] = 0;


//            printf("Client(%d)\n", client->newsockfd);
//            printf("Pouzil prikaz: %s\n", command);
//            printf("Pre osobu: %s\n", user);
//            printf("S obsahom: %s\n", text);

            FILE *fptr;
            fptr = fopen("/home/pos/msgLog.txt", "a");
            if (fptr == NULL) {
                printf("Error! neviem otvorit subor msgLog.\n");
                break;
            }

            char tmp[256];
            bzero(tmp, 256);
            strcat(tmp, "n ");
            strcat(tmp, client->name);
            strcat(tmp, ": ");
            strcat(tmp, text);


            // x je cislo na ktory socket treba poslat spravu
            // na zaciatku je nastaveny samemu sebe
            int x = 0;
            for (int i = 4; i < data->size; i++) {
                if (!strcmp(data->client[i].name, user)) {
                    x = data->client[i].newsockfd;
                    jeOnline = true;
                    break;
                }
            }

            if(x == 0) {
                char* temp = "msg Tento uzivatel momentalne nieje online, precita si vasu spravu neskor";
                n = write(client->newsockfd, temp, strlen(temp));
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
            } else {
                n = write(x, tmp, strlen(tmp));
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
            }

            char fetak[300];
            bzero(fetak, 300);

            if(jeOnline){
                strcat(fetak, "online ");
            } else {
                strcat(fetak, "offline ");
            }
            strcat(fetak, "n ");
            strcat(fetak, client->name);
            strcat(fetak, " ");
            strcat(fetak, name);
            strcat(fetak, " ");
            strcat(fetak, text);
            fprintf(fptr, fetak);
            fclose(fptr);

        } else if (!strcmp(command, "quit")) {
            n = write(client->newsockfd, "terminujem ta", 14);
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
            client->name = "*";
            close(client->newsockfd);
            client->newsockfd = 0;
            break;
        } else if (!strcmp(command, "msgC")) {
            user = strtok(NULL, " ");
            char *token = strtok(NULL, " ");
            bzero(text, 201);

            while (token != NULL) {
                strcat(text, token);
                strcat(text, " ");
                token = strtok(NULL, " ");
            }
            text[strlen(text) - 1] = 0;


//            printf("Client(%d)\n", client->newsockfd);
//            printf("Pouzil prikaz: %s\n", command);
//            printf("Pre osobu: %s\n", user);
//            printf("S obsahom: %s\n", text);

            FILE *fptr;
            fptr = fopen("/home/pos/msgLog.txt", "a");
            if (fptr == NULL) {
                printf("Error! neviem otvorit subor msgLog.\n");
                break;
            }
            char txtt[300];
            bzero(txtt, 300);
            strcat(txtt, "s ");
            strcat(txtt, client->name);
            strcat(txtt, " ");
            strcat(txtt, user);
            strcat(txtt, " ");
            strcat(txtt, text);

            fprintf(fptr, txtt);
            fclose(fptr);

            char tmp[256];
            bzero(tmp, 256);
            strcat(tmp, "s ");
            strcat(tmp, client->name);
            strcat(tmp, ": ");
            strcat(tmp, text);

            int x = client->newsockfd;
            for (int i = 4; i < data->size; i++) {
                if (!strcmp(data->client[i].name, user)) {
                    x = data->client[i].newsockfd;
                    break;
                }
            }
            n = write(x, tmp, strlen(tmp));// +1 za
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
        } else if (!strcmp(command, "show")) {
            char *temp = "*";
            char finall[300];
            bzero(finall, 300);
            strcat(finall, "show ");

            for (int i = 4; i < data->size; i++) {
                if (strcmp(data->client[i].name, temp)) {
                    strcat(finall, data->client[i].name);
                    strcat(finall, " ");
                }
            }
            n = write(client->newsockfd, finall, strlen(finall));
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
        } else if (!strcmp(command, "add")) {
            //todo checkni ci uz nahodou neni jeho friend predtym ako posles ziadost o nove priatelstvo
            user = strtok(NULL, " ");
            user[strlen(user) - 1] = 0;
            int x = 0;
            for (int i = 4; i < data->size; i++) {
                if (!strcmp(data->client[i].name, user)) {
                    x = data->client[i].newsockfd;
                    break;
                }
            }
            if (x != 0) {
                char *temp1 = "friendRequest ";
                char temp2[strlen(temp1) + strlen(client->name)];
                bzero(temp2, strlen(temp1) + strlen(client->name));
                strcat(temp2, temp1);
                strcat(temp2, client->name);
                n = write(x, temp2, strlen(temp2));
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
                data->client[x].request = client;
            } else {
                n = write(client->newsockfd, "Tento uzivatel nieje prihlasny!\n", 33);
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);
                }
            }
        } else if(!strcmp(command, "decline")){
            char str[300];
            bzero(str,300);
            strcat(str, "noFriendsForYou");
            strcat(str, " ");
            strcat(str, client->name);
            n = write(client->newsockfd, str, strlen(str));
            if (n < 0) {
                perror("Error writing to socket");
                exit(5);
            }
            client->request = NULL;

        }else if (!strcmp(command, "unfriend")) {
            //todo milan sem treba zabezpecit ze ak je typek offline tak aby mu prisla info msg ze ho uz ludia nemaju radi

            char* badFriend = "";
            badFriend = strtok(NULL, " ");

            if(badFriend[strlen(badFriend) - 1] == '\n') {
                badFriend[strlen(badFriend) - 1] = 0;
            }

            FILE* fptr;
            FILE* fakefptr;
            fptr = fopen("/home/pos/friendData.txt","r");
            if (!fptr) {
                printf("neviem otvorit friendData.txt!\n");
                return 1;
            }

            fakefptr = fopen("/home/pos/tmp.txt", "w");
            if (!fakefptr){
                printf("neviem otvorit tmp file!!\n");
                return 1;
            }

            char line[300];
            char* line2;
            bzero(line, 300);

            while(fgets(line, sizeof(line), fptr)){
                line2 = strdup(line);
                char* tname = "";
                tname = strtok(line, " ");
                char priatelia[300];
                bzero(priatelia, 300);
                if(!strcmp(tname, client->name)){
                    //nasiel riadok kde je typek ktory unfrienduje
                    int pocetFriendovAkceptujuci = atoi(strtok(NULL, " "));

                    if(pocetFriendovAkceptujuci > 0) {
                        strcat(priatelia, " ");

                        for(int i=0; i < pocetFriendovAkceptujuci ;i++){
                            char* najnovsi = "";
                            if(i == pocetFriendovAkceptujuci-1){
                                najnovsi = strtok(NULL, " ");
                                if(najnovsi == NULL) {
                                    break;
                                }
                                if(najnovsi[strlen(najnovsi) - 1] == '\n') {
                                    najnovsi[strlen(najnovsi) - 1] = 0;
                                }
                                if(!strcmp(najnovsi, badFriend)){

                                }else{
                                    if(najnovsi == NULL) {
                                        break;
                                    }
                                    if(najnovsi[strlen(najnovsi) - 1] == '\n') {
                                        najnovsi[strlen(najnovsi) - 1] = 0;
                                    }
                                    strcat(priatelia, najnovsi);
                                }
                            }else{
                                najnovsi = strtok(NULL, " ");
                                if(!strcmp(najnovsi, badFriend)){

                                } else{
                                    if(najnovsi == NULL) {
                                        break;
                                    }
                                    strcat(priatelia, najnovsi);
                                    strcat(priatelia, " ");
                                }
                            }
                        }
                    }else{
                        n = write(client->newsockfd, "Nemas ziadnych kamaratov\n", 20);
                        if (n < 0) {
                            perror("Error writing to socket");
                            exit(5);
                        }
                        continue;
                    }

                    char cislo[10];
                    bzero(cislo,10);
                    char totmp[500];
                    bzero(totmp, 500);
                    strcat(totmp, tname);
                    strcat(totmp, " ");
                    sprintf(cislo, "%d", pocetFriendovAkceptujuci-1);
                    strcat(totmp, cislo);
                    strcat(totmp, priatelia);
                    strcat(totmp, "\n");//potencionalne zbytocne

                    fprintf(fakefptr, totmp);

                }else if(!strcmp(tname, badFriend)){
                    //nasiel riadok cloveka ktory je mazany
                    int pocetFriendov = atoi(strtok(NULL, " "));

                    if(pocetFriendov > 0) {
                        strcat(priatelia, " ");

                        for(int i=0; i < pocetFriendov ;i++){
                            char* najnovsi = "";
                            if(i == pocetFriendov-1){

                                najnovsi = strtok(NULL, " ");
                                if(najnovsi[strlen(najnovsi) - 1] == '\n') {
                                    najnovsi[strlen(najnovsi) - 1] = 0;
                                }
                                if(!strcmp(najnovsi, client->name)){

                                }else{
                                    if(najnovsi == NULL) {
                                        break;
                                    }
                                    strcat(priatelia, najnovsi);
                                    priatelia[strlen(priatelia)-1] = 0;
                                }
                            }else{
                                najnovsi = strtok(NULL, " ");
                                if(!strcmp(najnovsi, client->name)){

                                } else{
                                    if(najnovsi == NULL) {
                                        break;
                                    }
                                    if(najnovsi[strlen(najnovsi) - 1] == '\n') {
                                        najnovsi[strlen(najnovsi) - 1] = 0;
                                    }
                                    strcat(priatelia, najnovsi);
                                    strcat(priatelia, " ");
                                }
                            }
                        }
                    }else{
                        continue;
                    }


                    char cislo[10];
                    bzero(cislo,10);
                    char totmp[500];
                    bzero(totmp, 500);
                    strcat(totmp, tname);
                    strcat(totmp, " ");
                    sprintf(cislo, "%d", pocetFriendov-1);
                    strcat(totmp, cislo);
                    strcat(totmp, priatelia);
                    strcat(totmp, "\n");//potencionalne zbytocne

                    fprintf(fakefptr, totmp);
                } else{
                    fprintf(fakefptr, line2);
                }
            }
            // v tomto bode mam napisane v tmp.txt riadky z originalu obohatene o noveho curaka a incrementnute mnozstvo


            //koniec bs prepisu
            fclose(fakefptr);
            fclose(fptr);

            if (remove("/home/pos/friendData.txt") == 0) {
                printf("The file friendData is deleted successfully.\n");
            } else {
                printf("The file friendData.txt is not deleted.\n");

            }
            int resulttt = rename("/home/pos/tmp.txt", "/home/pos/friendData.txt");
            if (resulttt == 0) {
                printf("The file tmp.txt is renamed successfully to friendData.\n");
            } else {
                printf("The file tmp.txt could not be renamed.\n");
            }

            bool posuvaj = false;
            for(int i=0; i < 50; i++){
                if(posuvaj){
                    client->friends[i-1] = client->friends[i];
                }
                if(client->friends[i].name == badFriend){
                    client->friends[i].name = "*";
                    bool posuvaj = true;
                }
            }
            client->nastaloMazanie = true;


            Client* docasny;
            for(int i = 0; i < data->size; i++){
                if(!strcmp(data->client[i].name, badFriend)){
                    docasny = data->client+i;
                }
            }
            posuvaj = false;
            for(int i=0; i < 50; i++){
                if(posuvaj){
                    docasny->friends[i-1] = docasny->friends[i];
                }
                if(docasny->friends[i].name == client->name){
                    docasny->friends[i].name = "*";
                    bool posuvaj = true;
                }
            }



        } else if (!strcmp(command, "accept")) {
            if(client->request == NULL) {
                n = write(client->newsockfd, "Nie je koho pridat!\n", 20);
                if (n < 0) {
                    perror("Error writing to socket");
                    exit(5);

                }
                continue;
            }
            //todo fucking file to file line altering BS
            FILE* fptr;
            FILE* fakefptr;
            fptr = fopen("/home/pos/friendData.txt","r");
            if (!fptr) {
                printf("Unable to open the original friendData file!!\n");
                return 0;
            }

            fakefptr = fopen("/home/pos/tmp.txt", "w");
            if (!fakefptr) {
                printf("Unable to open the tmp file!!\n");
                return 0;
            }

            char line[300];
            char* line2;
            bzero(line, 300);

            while(fgets(line, sizeof(line), fptr)){
                line2 = strdup(line);
                char* tname = "";
                tname = strtok(line, " ");
                char priatelia[300];
                bzero(priatelia, 300);
                if(!strcmp(tname, client->name)){
                    //nasiel riadok kde je typek ktory akceptuje
                    int pocetFriendovAkceptujuci = atoi(strtok(NULL, " "));

                    if(pocetFriendovAkceptujuci > 0) {
                        strcat(priatelia, " ");

                        for(int i=0; i < pocetFriendovAkceptujuci ;i++){
                            if(i == pocetFriendovAkceptujuci-1){
                                strcat(priatelia, strtok(NULL, " "));
                                priatelia[strlen(priatelia)-1] = 0;
                            }else{
                                strcat(priatelia, strtok(NULL, " "));
                                strcat(priatelia, " ");
                            }
                        }
                    }
                    char cislo[10];
                    bzero(cislo,10);
                    char totmp[500];
                    bzero(totmp, 500);
                    strcat(totmp, tname);
                    strcat(totmp, " ");
                    sprintf(cislo, "%d", pocetFriendovAkceptujuci+1);
                    strcat(totmp, cislo);
                    strcat(totmp, priatelia);
                    strcat(totmp, " ");
                    strcat(totmp, client->request->name);
                    strcat(totmp, "\n");//potencionalne zbytocne

                    fprintf(fakefptr, totmp);


                }else if(!strcmp(tname, client->request->name)) {
                    //nasiel riadok kde je typek ktory ziada
                    int pocetFriendovZiadajuci= atoi(strtok(NULL, " "));
                    if(pocetFriendovZiadajuci > 0) {
                        strcat(priatelia, " ");

                        for(int i=0; i < pocetFriendovZiadajuci ;i++) {
                            if(i == pocetFriendovZiadajuci-1){
                                strcat(priatelia, strtok(NULL, " "));
                                priatelia[strlen(priatelia)-1] = 0;
                            } else {
                                strcat(priatelia, strtok(NULL, " "));
                                strcat(priatelia, " ");
                            }
                        }
                    }
                    char cislo[10];
                    bzero(cislo,10);
                    char totmp[500];
                    bzero(totmp, 500);
                    strcat(totmp, tname);
                    strcat(totmp, " ");
                    sprintf(cislo, "%d", pocetFriendovZiadajuci+1);
                    strcat(totmp, cislo);
                    strcat(totmp, priatelia);
                    strcat(totmp, " ");

                    strcat(totmp, client->name);
                    strcat(totmp, "\n");//potencionalne zbytocne
                    fprintf(fakefptr, totmp);
                } else{
                    fprintf(fakefptr, line2);
                }
            }
            // v tomto bode mam napisane v tmp.txt riadky z originalu obohatene o noveho curaka a incrementnute mnozstvo


            //koniec bs prepisu
            fclose(fakefptr);
            fclose(fptr);

            if (remove("/home/pos/friendData.txt") == 0) {
                printf("The file friendData is deleted successfully.\n");
            } else {
                printf("The file friendData.txt is not deleted.\n");

            }
            int resulttt = rename("/home/pos/tmp.txt", "/home/pos/friendData.txt");
            if (resulttt == 0) {
                printf("The file tmp.txt is renamed successfully to friendData.\n");
            } else {
                printf("The file tmp.txt could not be renamed.\n");
            }

            for (int i = 0; i < 50; i++) {
                char * asshole;
                asshole = "*";
                if(!strcmp((client->friends[i].name), asshole)){
                    client->friends[i] = *client->request;

                    for(int j = 0; j < 50; j++){

                        if(!strcmp(data->client[client->request->newsockfd].friends[j].name, "*")){
                            data->client[client->request->newsockfd].friends[j] = *client;
                            break;
                        }
                    }
                    break;
                }
            }
            client->request = NULL;
        }  else if (!strcmp(command, "createGroup")) {
            char* groupName = strtok(NULL, " ");
            char* pocet = strtok(NULL, " ");
            char* user = "";
            char temp[300];
            bzero(temp, 300);
            FILE *fptr;
            char final[300];
            bzero(final, 300);

            user = strtok(NULL, " ");
            int x = atoi(pocet);
            for(int i = 0; i < x; i++) {

                strcat(temp, user);

                if(i+1 == pocet) {
                    user[strlen(user) - 1] = 0;
                } else {
                    strcat(temp, " ");
                }

                user = strtok(NULL, " ");
            }
            fptr = fopen("/home/pos/groupData.txt", "a");
            strcat(final, groupName);
            strcat(final, " ");
            strcat(final, pocet);
            strcat(final, " ");

            strcat(final, temp);
            final[strlen(final) - 1] = 0;
            fprintf(fptr, final);
            fclose(fptr);
        } else if (!strcmp(command, "msgg")) {
            FILE *fptr;
            fptr = fopen("/home/pos/groupData.txt", "r");
            char line[256];
            char* groupName = "";
            int pocet = 0;
            char* temp = "";
            char* groupToMSG = "";
            groupToMSG = strtok(NULL, " ");
            char text[300];
            bzero(text, 300);
            char* word = "";
            char final[300];
            bzero(final, 300);

            word = strtok(NULL, " ");
            strcat(text, word);
            while(1) {
                strcat(text, " ");
                word = strtok(NULL, " ");
                if(word == NULL) {
                    break;
                }
                strcat(text, word);
            }


            strcat(final, "msgg ");
            strcat(final, groupToMSG);
            strcat(final, " ");
            strcat(final, client->name);
            strcat(final, " ");
            strcat(final, text);

            while (fgets(line, sizeof(line), fptr)) {
                groupName = strtok(line, " ");
                if(!strcmp(groupName, groupToMSG)) {
                    pocet = atoi(strtok(NULL, " "));
                    for(int i = 0; i < pocet; i++) {
                        temp = strtok(NULL, " ");
                        if(i+1 == pocet) {
                            temp[strlen(temp) - 1] = 0;
                        }
                        for (int j = 0; j < data->size; j++) {
                            if(!strcmp(data->client[j].name, temp)) {
                                if(strcmp(temp, client->name)) {
                                    n = write(data->client[j].newsockfd, final, strlen(final));
                                    if (n < 0) {
                                        perror("Error writing to socket");
                                        exit(5);
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            fclose(fptr);

            FILE *fptr2;
            fptr2 = fopen("/home/pos/groupLog.txt", "a");
            if (fptr2 == NULL) {
                printf("Error! neviem otvorit subor groupLog.\n");
                break;
            }
            final[strlen(final) - 1] = 0;
            fprintf(fptr2, final);
            fclose(fptr2);
        } else if (!strcmp(command, "history")) {
            char line[300];
            FILE *fptr;
            fptr = fopen("/home/pos/msgLog.txt", "r");
            if (fptr == NULL) {
                printf("Error! neviem otvorit subor msgLog.\n");
                break;
            }
            char* stav = "";
            char* sifra = "";
            char* odosielatel = "";
            char* prijemca = "";
            char sprava[300];
            char final[300];

            while (fgets(line, sizeof(line), fptr)) {
                bzero(sprava, 300);
                bzero(final, 300);
                stav = strtok(line, " ");
                sifra = strtok(NULL, " ");
                odosielatel = strtok(NULL, " ");
                prijemca = strtok(NULL, " ");
                while(1) {
                    char* temp = "";
                    temp = strtok(NULL, " ");
                    if(temp == NULL) {
                        sprava[strlen(sprava) - 1] = 0;
                        break;
                    } else {
                        strcat(sprava, temp);
                        strcat(sprava, " ");
                    }
                }

                if(!strcmp(prijemca, client->name)) {
                    strcat(final, sifra);
                    strcat(final, " ");
                    strcat(final, odosielatel);
                    strcat(final, ": ");
                    strcat(final, sprava);

                    n = write(client->newsockfd, final, strlen(final));
                    if (n < 0) {
                        perror("Error writing to socket");
                        exit(5);
                    }

                    sleep(1);
                }
            }
            fclose(fptr);
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
    serv_addr.sin_port = htons(portt);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        portt++;
        serv_addr.sin_port = htons(portt);
        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Error binding socket address");
            exit(2);
        }
    }
    printf("Server bezi na porte: %d\n",portt);

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


int main(int argc, char *argv[]) {
    //todo check if the txt files are created in correct path
    printf("startujem main\n");

    FILE *file;
    file = fopen("milanko.txt","a+");
    fprintf(file,"ahoj %i\n",16);


    fclose(file);
    printf("koniec milanko\n");

    if( access( "/userData.txt", F_OK ) == 0 ) {
        printf("userData ok\n");
    } else {
        printf("nema access k userData\n");
        file = fopen("/userData.txt", "r");
        printf("nema access k userData po fopne\n");
        fclose(file);
    }
    if( access( "/groupData.txt", F_OK ) == 0 ) {
    } else {
        file = fopen("/groupData.txt", "r");
        fclose(file);
    }
    printf("groupData finished\n");
    if( access( "friendData.txt", F_OK ) == 0 ) {
    } else {
        file = fopen("friendData.txt", "r");
        fclose(file);
    }
    if( access( "msgLog.txt", F_OK ) == 0 ) {
    } else {
        file = fopen("msgLog.txt", "r");
        fclose(file);
    }
    if( access( "groupLog.txt", F_OK ) == 0 ) {
    } else {
        file = fopen("groupLog.txt", "r");
        fclose(file);
    }
    printf("skoncila inicializacia suborov \n");
    pthread_mutex_t mutex;
    pthread_cond_t cGenerate;
    pthread_cond_t cPrint;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cGenerate, NULL);
    pthread_cond_init(&cPrint, NULL);

    pthread_t printer;
    pthread_t tlistener;

    Client client[100];
    for (int i = 0; i < 100; ++i) {
        client[i].name = "*";
    }

    Data data;
    data.index=0;
    data.size=100;
    data.client = client;
    data.mutex = &mutex;

    pthread_create(&printer, NULL, print, &data);
    pthread_create(&tlistener, NULL, listener, &data);

    pthread_join(printer, NULL);
    pthread_join(tlistener, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cGenerate);
    pthread_cond_destroy(&cPrint);

    return 0;
}