#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define BUFFER_SIZE 4096

void log_msg(char *msg, char *path) {
    FILE *file = fopen(path, "a, ccs=UTF-8");
    if (file == NULL) {
        printf("\n============ ADVERTENCIA ============\n");
        printf("No se pudo abrir el archivo de log!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s", msg);
    fclose(file);
}

void response(int socket, int status, char *status_msg, char *content_type, char *filename, char *body) {
    char buffer[BUFFER_SIZE] = {0};
    sprintf(buffer, "HTTP/1.1 %d %s\nContent-Type: %s\n", status, status_msg, content_type);
    send(socket, buffer, strlen(buffer), 0);
    char *extension = strrchr(filename, '.');
    if (extension != NULL) {
        extension++; // saltar el punto
        if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0 || strcmp(extension, "png") == 0 || strcmp(extension, "gif") == 0) {
            // Si el contenido es una imagen, enviarla binariamente
            FILE *file = fopen(filename, "rb");
            if (file == NULL) {
                printf("Error al abrir el archivo de imagen: %s\n", filename);
                exit(EXIT_FAILURE);
            }
            int bytes_read = 0;
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                send(socket, buffer, bytes_read, 0);
            }
            fclose(file);
            return;
        }
    }
    // Si el contenido no es una imagen, enviar el cuerpo de la respuesta
    sprintf(buffer, "%s", body);
    send(socket, buffer, strlen(buffer), 0);
}


int main(int argc, char const *argv[]) {

    setlocale(LC_ALL, "es_ES.utf8");

    if (argc != 4) {
        printf("Error: Debe ingresar el puerto, el path del archivo de log y el path del archivo de respuesta\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    char *path = (char *)argv[2];

    char *folder = (char *)argv[3];

    int server_fd, new_socket;

    struct sockaddr_in address;

    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creando el socket del servidor :(");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;

    address.sin_addr.s_addr = INADDR_ANY;

    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en el bind del socket del servidor :(");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Error en el listen del socket del servidor :(");
        exit(EXIT_FAILURE);
    }

    while (1) {

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Error en el accept del socket del servidor :(");
            exit(EXIT_FAILURE);
        }

        int valread = read(new_socket, buffer, BUFFER_SIZE);

        printf("Recibido: %s\n", buffer);

        log_msg(buffer, path);

        char *method = strtok(buffer, " ");
        char *path = strtok(NULL, " ");

        char *full_path = malloc(strlen(folder) + strlen(path) + 1);

        strcpy(full_path, folder);
        strcat(full_path, path);

    if (strcmp(method, "GET") == 0) {
        printf("El método es GET\n");

        FILE *res_file = fopen(full_path, "r");

        if (res_file == NULL) {
            response(new_socket, 404, "Not Found", "text/html", NULL, "<h1>404 No se encontró lo que está buscando pa :(</h1>");
        } else {
            char *file_content = malloc(BUFFER_SIZE);
            int file_size = fread(file_content, 1, BUFFER_SIZE, res_file);
            response(new_socket, 200, "OK", "text/html", full_path, file_content);
            free(file_content);
            fclose(res_file);
        }
    } else {
        response(new_socket, 405, "Method Not Allowed", "text/html", NULL, "<h1>405 Método no permitido</h1>");
    }

        close(new_socket);

    }

    return 0;
}
