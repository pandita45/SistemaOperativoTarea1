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

    for(int i = 0; i <= pipes; i++){

        if (strcmp(argv[i][0], "help") == 0) {
            printf("  exit   - Cierra la shell\n");
            printf("  help   - Muestra esta ayuda\n");
            printf("  miprof - Mide el tiempo de ejecucion de un comando\n");
            continue; 
        }
        if(strcmp(argv[i][0], "miprof") == 0){
            if(strcmp(argv[i][1], "ejec") == 0  || argv[i][1] == NULL){
                if(argv[i][2] == NULL){
                    fprintf(stderr, "Error: Use 'miprof help', para mas informacion.\n");
                    return 0;
                }
                else{
                    // eliminar "miprof" y "ejec" y que quede solo el comando a ejecutar 
                    int j;  
                    for(j = 0; argv[i][j + 2] != NULL; j++){
                        argv[i][j] = argv[i][j + 2];
                    }
                    argv[i][j] = NULL; 
                    struct timeval start, end;
                    struct rusage usage;

                    gettimeofday(&start, NULL);
                    
                    pid_t pid = fork();
                    if (pid == 0) {
                        execvp(argv[i][0], argv[i]);
                        fprintf(stderr, "Comando no encontrado: %s \n Pruebe usando 'help'\n", argv[i][0]);
                        exit(1);
                    }
                    int status;
                    waitpid(pid, &status, 0);
                    //termina el tiempo de uso del proceso hijo
                    gettimeofday(&end, NULL);
                    //Obtener el uso de recursos del proceso hijo
                    getrusage(RUSAGE_CHILDREN, &usage);

                    double tiempoSistema = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
                    double tiempoUsuario = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
                    double tiempoReal = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

                    printf("Tiempo de usuario: %.6f segundos\n", tiempoUsuario);
                    printf("Tiempo de sistema: %.6f segundos\n", tiempoSistema);
                    printf("Tiempo real: %.6f segundos\n", tiempoReal);
                    continue;
                }

            } 
            else if(strcmp(argv[i][1], "help") == 0){
                printf("miprof ejec <comando>\n");
                printf("Ej: miprof ejec ls -l\n");
                continue;
            } else if (strcmp(argv[i][1], "ejecsave") == 0) {
                if (argv[i][2] == NULL || argv[i][3] == NULL) { // necesita al menos un comando y un archivo
                    fprintf(stderr, "Error: Use 'miprof help', para mas informacion.\n");
                    return 0;
                } else {
                    // eliminar "miprof" y "ejecsave" y que quede solo el comando a ejecutar 
                    int j;  
                    for (j = 0; argv[i][j + 2] != NULL; j++) {
                        argv[i][j] = argv[i][j + 2]; // desplazamos todo a la izquierda
                    }

                    argv[i][j] = NULL; // terminador NULL para execvp
                    struct timeval start, end; // Variables para medir el tiempo
                    struct rusage usage; // Variable para obtener el uso de recursos

                    gettimeofday(&start, NULL); // Inicia el tiempo
                    
                    pid_t pid = fork(); // Crear un proceso hijo
                    if (pid == 0) { // Proceso hijo
                        execvp(argv[i][0], argv[i]); // Ejecuta el comando
                        fprintf(stderr, "Comando no encontrado: %s \n Pruebe usando 'help'\n", argv[i][0]); // Si exec falla
                        exit(1); // Salir del proceso hijo
                    }

                    int status;
                    waitpid(pid, &status, 0);
                    //termina el tiempo de uso del proceso hijo
                    gettimeofday(&end, NULL);
                    //Obtener el uso de recursos del proceso hijo
                    getrusage(RUSAGE_CHILDREN, &usage);

                    double tiempoSistema = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
                    double tiempoUsuario = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
                    double tiempoReal = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

                    FILE *file = fopen(argv[i][j-1], "a"); // Abrir el archivo en modo append
                    if (file == NULL) {
                        fprintf(stderr, "Error al abrir el archivo %s para escribir.\n", argv[i][j-1]);
                        return 1;
                    }

                    fprintf(file, "Tiempo de usuario: %.6f segundos\n", tiempoUsuario); // Guardar en el archivo
                    fprintf(file, "Tiempo de sistema: %.6f segundos\n", tiempoSistema); // Guardar en el archivo
                    fprintf(file, "Tiempo real: %.6f segundos\n", tiempoReal); // Guardar en el archivo
                    fclose(file); // Cerrar el archivo
                    continue;
                }
            } else {
                fprintf(stderr, "Error: Use 'miprof help', para mas informacion.\n");
                return 0;
            }
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
            execvp(argv[i][0], argv[i]); 
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

    while (manejarInput(i) != -1) {
        printf(PROMPT);
        fgets(i, BUFSIZ, stdin);
    }
    
    printf("Saliendo...\n");
    return 0;
}
