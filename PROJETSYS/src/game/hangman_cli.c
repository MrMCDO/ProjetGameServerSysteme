#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../libs/libmessage/message.h"
#include "../libs/libprojectUtil/projectUtil.h"

#define SERV_IN_FILENO 4
#define SERV_OUT_FILENO 3
#define MSG_ERROR_COMM "Error in communication with the server."
#define ERROR_CODE_COMM  63
#define PSEUDO_MAX_SIZE 10

char *string = NULL;
int pid_serv;

void all_destroy(void) {
    if(string != NULL) {
        free(string);
    }
}

void emptyBuffer(){
    int c = 0;
    while (c != '\n' && c != EOF){
        c = getchar();
    }
}

int verifySizePseudo(char *pseudo){
    int i = 0;
    for( i = 0 ; i <= PSEUDO_MAX_SIZE ; i++){
        if(pseudo[i] == '\n'){
            pseudo[i] = '\0';
            return 1;
        }
    }
    return 0;
}

//handler for SIGALRM
void handler_alrm(int sig){
    fprintf(stdout, "\nYou are out of time\n");
    all_destroy();
    exit(0);
}

//handler for sigUSR2
void handler_usr2(int sig){
    fprintf(stderr, "\nThe server is down\nExit\n");
    all_destroy();
    exit(87);
}

//handler for sigINT
void handler_int(int sig){
    fprintf(stdout, "\nExit\n");
    if(kill(pid_serv, SIGUSR2) == -1){
        perror("kill");
        all_destroy();
        exit(97);
    }
    all_destroy();
    exit(0);
}

int main(int argc, char **argv){
    struct sigaction saUSR2;
    struct sigaction saInt;
    saUSR2.sa_handler = handler_usr2;
    saInt.sa_handler = handler_int;
    sigemptyset(&saInt.sa_mask);
    sigemptyset(&saUSR2.sa_mask);
    saUSR2.sa_flags = 0;
    saInt.sa_flags = 0;
    if(sigaction(SIGUSR2, &saUSR2, NULL) == -1){
        perror("sigaction USR2");
        exit(27);
    }
    if(sigaction(SIGINT, &saInt, NULL) == -1){
        perror("sigaction INT");
        exit(27);
    }
    pid_serv = recv_int(SERV_IN_FILENO);
    if(pid_serv == -1){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        all_destroy();
        exit(61);
    }
    // server indicates if arguments are valid or not
    int valid_argv = recv_int(SERV_IN_FILENO);
    if (valid_argv == -1){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        all_destroy();
        exit(ERROR_CODE_COMM);
    }
    if (!valid_argv) {
        fprintf(stderr,"Les arguments ne sont pas valides.\n");
        char *error_msg = recv_string(SERV_IN_FILENO);
        if(error_msg == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM); 
        }
        fprintf(stderr, "%s\n", error_msg);
        all_destroy();
        exit(2);
    }
    //receive welcome message
    string = recv_string(SERV_IN_FILENO);
    if(string == NULL){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        exit(ERROR_CODE_COMM);
    }
    fprintf(stdout, "%s\n", string);
    free(string);
    string=NULL;
    // Receive number of tries message
    string = recv_string(SERV_IN_FILENO);
    if(string == NULL){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        exit(ERROR_CODE_COMM);
    }
    fprintf(stdout, "%s\n", string);
    free(string);
    string = NULL;
    //receive the timer msg
    string = recv_string(SERV_IN_FILENO);
    if(string == NULL){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        all_destroy();
        exit(ERROR_CODE_COMM);
    }
    fprintf(stdout, "%s\n", string);
    free(string);
    string = NULL;
    //receive the message of begin of the game
    string = recv_string(SERV_IN_FILENO);
    if(string == NULL){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        all_destroy();
        exit(ERROR_CODE_COMM);
    }
    fprintf(stdout, "%s\n", string);
    free(string);
    string=NULL;
    char char_input;
    int input_ok;
    int bool_again = 1;
    int nb_choice = 1;

    struct sigaction act;
    act.sa_handler = handler_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) == -1) {
       perror("sigaction");
        all_destroy();
        exit(1);
    }

    while(bool_again){
        //receive the current display of the word
        string = recv_string(SERV_IN_FILENO);
        if(string == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM);
        }
        fprintf(stdout, "%s\n", string);
        free(string);
        string=NULL;
        input_ok = 0;
        while(input_ok == 0){
            fprintf(stdout, "Choice %d, enter a letter :", nb_choice);
            char_input = getchar();
            emptyBuffer();
            if(char_input == '\n'){
                fprintf(stdout, "Invalid letter.\n");
                continue;
            }
            if(send_char(SERV_OUT_FILENO, char_input) == -1){
                fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                all_destroy();
                exit(ERROR_CODE_COMM);
            }
            input_ok = recv_int(SERV_IN_FILENO);
            if(input_ok == -1){
                fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                all_destroy();
                exit(ERROR_CODE_COMM);
            }else if(input_ok == 0){
                fprintf(stderr, "Invalid letter. Retry.\n");
            }
        }
        nb_choice++;

        //receive the answer of the server
        string = recv_string(SERV_IN_FILENO);
        if(string == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM);
        }
        fprintf(stdout, "%s\n", string);
        free(string);
        string=NULL;
        bool_again = recv_int(SERV_IN_FILENO);
        if(bool_again == -1){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM);
        }
    }
    //receive the information of the result
    int bool_result = recv_int(SERV_IN_FILENO);
    if(bool_result == -1){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        all_destroy();
        exit(ERROR_CODE_COMM);
    }
    //receive the message of end of the game
    string = recv_string(SERV_IN_FILENO);
    if(string == NULL){
        fprintf(stderr, "%s\n",MSG_ERROR_COMM);
        exit(ERROR_CODE_COMM);
    }
    fprintf(stdout, "%s\n", string);
    free(string);
    string = NULL;
    if(bool_result){
        string = recv_string(SERV_IN_FILENO);
        if(string == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            exit(ERROR_CODE_COMM);
        }
        fprintf(stdout, "%s\n", string);
        free(string);
        string = NULL;
        char_input = getchar();
        emptyBuffer();
        while(char_input != 'y' && char_input != 'n' && char_input != 'Y' && char_input != 'N'){
            fprintf(stdout, "Error in the input. Must be (y/Y or n/N)\n");
            char_input = getchar();
            emptyBuffer();
        }
        if(send_char(SERV_OUT_FILENO, char_input) != 0){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM);
        }
        if(char_input == 'n' || char_input == 'N'){
            return 0;
        }
        string = recv_string(SERV_IN_FILENO);
        if(string == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            exit(ERROR_CODE_COMM);
        }
        fprintf(stdout, "%s\n", string);
        free(string);
        string = NULL;
        bool_again = 1;
        while(bool_again){
            string = recv_string(SERV_IN_FILENO);
            if(string == NULL){
                fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                exit(ERROR_CODE_COMM);
            }
            fprintf(stdout, "%s", string);
            free(string);
            string=NULL;
            string = calloc(sizeof(char), PSEUDO_MAX_SIZE+1);
            if(fgets(string, PSEUDO_MAX_SIZE, stdin) == NULL){;
                fprintf(stderr, "Error with the input.\n");
                perror("fgets");
                all_destroy();
                exit(15);
            }
            while(verifySizePseudo(string) == 0){
                emptyBuffer();
                fprintf(stdout, "Error in the input. Must be between 4 and %d characters.\n", PSEUDO_MAX_SIZE);
                fprintf(stdout, "Please retry : ");
                if(fgets(string, PSEUDO_MAX_SIZE, stdin) == NULL){
                    fprintf(stderr, "Error with the input.\n");
                    perror("fgets");
                    all_destroy();
                    exit(15);
                }
            }
            if(send_string(SERV_OUT_FILENO, string) != 0){
                fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                all_destroy();
                exit(ERROR_CODE_COMM);
            }
            bool_again = recv_int(SERV_IN_FILENO);
            if(bool_again == -1){
                fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                exit(ERROR_CODE_COMM);
            }
            free(string);
            string=NULL;
            if(bool_again){
                string = recv_string(SERV_IN_FILENO);
                if(string == NULL){
                    fprintf(stderr, "%s\n",MSG_ERROR_COMM);
                    all_destroy();
                    exit(ERROR_CODE_COMM);
                }
                fprintf(stdout, "%s\n", string);
                free(string);
                string=NULL;
            }

        }
        string = recv_string(SERV_IN_FILENO);
        if(string == NULL){
            fprintf(stderr, "%s\n",MSG_ERROR_COMM);
            all_destroy();
            exit(ERROR_CODE_COMM);
        }
        fprintf(stdout, "%s\n", string);
        free(string);
        string=NULL;
    }
    return 0;
}   