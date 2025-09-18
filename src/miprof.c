#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

void exitProgram(){
    printf("time elapsed\n");
    exit(0);
}

int main(int argc, char *argv[]) {

    if (argv[1] == NULL || strcmp(argv[1], "help") == 0) {
        printf("Uso:\n");
        printf("  miprof ejec <comando>\n");
        printf("  miprof ejecsave <archivo> <comando>\n");
        printf("  miprof ejecutar <tiempo> <comando>\n");
        return 0;
    }

    struct timeval start, end;
    struct rusage usage;

    if (strcmp(argv[1], "ejec") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Error: Falta el comando a ejecutar.\n");
            return 1;
        }
        char **cmd = &argv[2];

        gettimeofday(&start, NULL);
        pid_t pid = fork();
        if (pid == 0) {
            execvp(cmd[0], cmd);
            fprintf(stderr, "Comando no encontrado: %s\n", cmd[0]);
            exit(1);
        }

        int status;
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        getrusage(RUSAGE_CHILDREN, &usage);

        double tUser = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double tSys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        double tReal = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

        printf("Tiempo de usuario: %.6f s\n", tUser);
        printf("Tiempo de sistema: %.6f s\n", tSys);
        printf("Tiempo real: %.6f s\n", tReal);
        return 0;
    }

    if (strcmp(argv[1], "ejecsave") == 0) {
        if (argv[2] == NULL || argv[3] == NULL) {
            fprintf(stderr, "Error: Falta archivo o comando.\n");
            return 1;
        }

        char *filename = argv[2];
        char **cmd = &argv[3];

        gettimeofday(&start, NULL);
        pid_t pid = fork();
        if (pid == 0) {
            execvp(cmd[0], cmd);
            fprintf(stderr, "Comando no encontrado: %s\n", cmd[0]);
            exit(1);
        }

        int status;
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        getrusage(RUSAGE_CHILDREN, &usage);

        double tUser = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double tSys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        double tReal = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

        FILE *f = fopen(filename, "a");
        if (!f) {
            perror("Error al abrir el archivo");
            return 1;
        }
        fprintf(f, "Tiempo de usuario: %.6f s\n", tUser);
        fprintf(f, "Tiempo de sistema: %.6f s\n", tSys);
        fprintf(f, "Tiempo real: %.6f s\n", tReal);
        fclose(f);
        return 0;
    }

    if(strcmp(argv[1],"ejecutar")==0){
        if(argv[2] == NULL || argv[3] == NULL){
            fprintf(stderr, "Falta comando o limite de tiempo (en segundos)");
        }

        char *timeLimit = argv[2];
        char **cmd = &argv[3];
        

        gettimeofday(&start, NULL);
        pid_t pid = fork();
        if (pid == 0) {
            struct sigaction sact;

            sigemptyset(&sact.sa_mask);
            sact.sa_flags = 0;
            sact.sa_handler = exitProgram;
            sigaction(SIGALRM,&sact,NULL);
            alarm(atoi(timeLimit));
            execvp(cmd[0], cmd);
            fprintf(stderr, "Comando no encontrado: %s\n", cmd[0]);
            exit(1);
        }

        int status;
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        getrusage(RUSAGE_CHILDREN, &usage);

        double tUser = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
        double tSys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
        double tReal = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

        printf("Tiempo de usuario: %.6f s\n", tUser);
        printf("Tiempo de sistema: %.6f s\n", tSys);
        printf("Tiempo real: %.6f s\n", tReal);
        return 0;

    }

    fprintf(stderr, "Subcomando desconocido. Use 'miprof help'.\n");
    return 1;
}

