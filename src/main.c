#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define BUFFER_SIZE BUFSIZ
#define PROMPT "shellao > "
#define MAXARGSIZE 10
#define MAXPIPES 15

char wd[8000];

int manejarInput(char* cadena){
    for(int i = 0; i < strlen(cadena); i++) if(cadena[i] == '\n') {cadena[i] = '\0'; break;}
    if (strcmp(cadena,"") == 0) return 0;
    if (strcmp(cadena,"exit") == 0) return -1;
    
    int it, pipes;
    for (it = 0, pipes = 0; cadena[it]; it++)
        pipes += (cadena[it] == '|');

    char* argv[pipes+1][MAXARGSIZE+1];
    char* tempPipes[pipes+1];
    char* comando;
    
    comando = strtok(cadena,"|");
    
    for(int i = 0; comando != NULL && i <= pipes; i++){
        tempPipes[i] = strdup(comando);
        comando = strtok(NULL,"|");
    }

    //nuestro arreglo tempPipes tiene cada comando individual, ahora podemos poblar argv

    for(int i = 0; i <= pipes; i++){
        char* ptr = tempPipes[i];
        while (*ptr==' ') ptr++; //quitamos los espacios al inicio de cada token
        char* token = strtok(ptr, " ");
        int argc;
        for(argc = 0; token != NULL && argc < MAXARGSIZE; argc++){
            argv[i][argc] = token;
            token = strtok(NULL, " "); // siguiente token
        }
        argv[i][argc] = NULL;
    }
    
    //ahora tenemos una matriz con cada argumento de cada comando
    //por lo que podemos crear los n procesos y pasar las n pipes
    int fd[pipes][2];
    pid_t child_pids[pipes+1];
    int pipeCount = 0;
    for(int i = 0; i <= pipes; i++){
        
        if(strcmp(argv[i][0], "cd") == 0){
            if(chdir(argv[i][1]) != 0){
                printf("Error al cambiar de directorio");
            }    
            continue;
        }

        if(i < pipes)
            pipe(fd[i]);


        child_pids[i] = fork();
        if (child_pids[i] == 0) {
            if (i > 0) {
                dup2(fd[i-1][0], 0);
            }
            if (i < pipes) {
                dup2(fd[i][1], 1);
            }
            for(int j = 0; j < pipes; j++){ 
                close(fd[j][0]);
                close(fd[j][1]);
            }

            if(strcmp(argv[i][0],"miprof") == 0){
                char initDir[8000];
                strcpy(initDir,wd);
                strcat(initDir,"/bin/miprof");

                execv(initDir, argv[i]);
            } else if(strcmp(argv[i][0],"help") == 0){
                char initDir[8000];
                strcpy(initDir,wd);
                strcat(initDir,"/bin/help");

                execv(initDir, argv[i]);
            } else {
                execvp(argv[i][0], argv[i]);
            }

            fprintf(stderr, "Comando no encontrado: %s \n Pruebe usando 'help'\n", argv[i][0]);
            exit(1);
        }
    }

    for(int j = 0; j < pipes; j++){
        close(fd[j][0]);
        close(fd[j][1]);
    }

    for(int i = 0; i <= pipes; i++)
        waitpid(child_pids[i], NULL, 0);

    for(int i = 0; i <= pipes; i++)
        free(tempPipes[i]);
}

int main(int argc, const char **argv) {
    char i[BUFFER_SIZE];
    printf(PROMPT);
    fgets(i, BUFSIZ, stdin);
    getcwd(wd,260*sizeof(char));
    while (manejarInput(i) != -1) {
        printf(PROMPT);
        fgets(i, BUFSIZ, stdin);
    }
    
    printf("Saliendo...\n");
    return 0;
}
