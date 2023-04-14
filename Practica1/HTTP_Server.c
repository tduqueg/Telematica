#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define BUFFER_SIZE 4096

void log_msg(char *msg, char * path) {

    FILE * file = fopen(path,"a, ccs=UTF-8" );
    if (file == NULL) {
        printf("\n============ ADVERTENCIA ============\n");
        printf("No se pudo abrir el archivo de log!!!\n");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s", msg);
    fclose(file);
}


void response(int socket, int status, char * status_msg, char * content, char * body){
    
    char buffer[BUFFER_SIZE] = {0};

    sprintf(buffer, "HTTP/1.1 %d %s\nContent-Type: %s\n\n%s", status, status_msg, content, body);
    send(socket, buffer, strlen(buffer), 0);
    printf("¡Respuesta enviada! buffer: %s\n", buffer);

}

int main(int argc, char const * argv[]){

    // Establecer la localidad
    setlocale(LC_ALL, "es_ES.utf8");

    // Validar argumentos de entrada del programa (puerto, path del archivo de log y path del archivo de respuesta)
    if( argc != 4){
        printf("Error: Debe ingresar el puerto, el path del archivo de log y el path del archivo de respuesta\n");
        exit(EXIT_FAILURE);
    
    }

    // Obtener el puerto del argumento de entrada
    int port = atoi(argv[1]);

    // Validar que el puerto sea un número válido
    char * path = (char * ) argv[2];

    // Validar que el path del archivo de log sea válido   
    char * folder = (char *) argv[3];

    // Declarar variables para el socket del servidor y el socket del cliente  
    int server_fd, new_socket;
    // Declarar variable para el valor de retorno de las funciones de lectura y escritura
    struct sockaddr_in address;
    // Declarar variable para el tamaño de la estructura de dirección
    int addrlen = sizeof(address);
    // Declarar el buffer para almacenar los datos recibidos del cliente (BUFFER_SIZE definido en 4096)
    char buffer[BUFFER_SIZE] = {0};


    // Crear el socket del servidor (ipv4, tcp, protocolo por defecto)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creando el socket del servidor :(");
        exit(EXIT_FAILURE);
    }

    // Asignar el puerto y la dirección al socket del servidor
    address.sin_family = AF_INET;
    // INADDR_ANY es una macro que se reemplaza por la dirección IP de la máquina
    address.sin_addr.s_addr = INADDR_ANY;
    // htons es una macro que convierte el puerto a formato de red
    address.sin_port = htons(port);
    // Si el bind falla, se imprime el error y se termina el programa
    if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0){
        perror("Error en el bind del socket del servidor :(");
        exit(EXIT_FAILURE);
    }


    // Escuchar conexiones entrantes en el socket del servidor (3 conexiones en espera)
    if (listen(server_fd, 3) < 0) {
        perror("Error en el listen del socket del servidor :(");
        exit(EXIT_FAILURE);
    }

    // Ciclo infinito para aceptar conexiones entrantes en el socket del servidor 
    while(1){

        // Aceptar una conexión entrante en el socket del servidor 
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("Error en el accept del socket del servidor :(");
            exit(EXIT_FAILURE);
        }

        // Leer los datos recibidos del cliente en el socket del cliente
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        // Imprimir los datos recibidos del cliente en el socket del cliente
        printf("Recibido: %s\n" , buffer);
        // Validar que se hayan recibido datos
        log_msg(buffer, path);

        // Obtener el método, la ruta y el protocolo del request
        char * method = strtok(buffer, " ");
        char * path = strtok(NULL, " ");
        // Obtener el path absoluto del archivo de respuesta
        char * full_path = malloc(strlen(folder) + strlen(path) + 1);
        // Concatenar el path del folder con el path del archivo de respuesta
        strcpy(full_path, folder);
        strcat(full_path, path);

        if(strcmp(method, "GET") == 0 ) {

            printf("El método es GET\n");
            // Abrir el archivo de respuesta
            FILE * res_file = fopen(full_path, "r");
            // Validar que el archivo de respuesta exista
            if (res_file == NULL){

                // Enviar una respuesta de error 404
                response(new_socket, 404, "Not Found", "text/html", "<h1>404 No se encontró lo que está buscando pa :(</h1>");

            } else {

                // Leer el archivo de respuesta
                if(strcmp(method, "GET") == 0){
                    
                    // Enviar una respuesta de éxito 200
                    printf("El método es GET y si se encontró y si se encontró :D\n");
                    // Obtener el tamaño del archivo de respuesta
                    char file_buffer[BUFFER_SIZE];
                    // Leer el archivo de respuesta y almacenarlo en el buffer 
                    int file_size = fread(file_buffer, 1, BUFFER_SIZE, res_file);
                    // Movemos el puntero al final del archivo
                    fseek(res_file, 0L, SEEK_END);
                    // Obtenemos la posición actual del puntero 
                    file_size = ftell(res_file);
                    // Movemos el puntero al inicio del archivo
                    fseek(res_file, 0L, SEEK_SET);
                    // Cerramos el archivo
                    fclose(res_file);
                    // Enviar la respuesta al cliente 
                    response(new_socket, 200, "OK", "text/html", file_buffer);
                }else{
                    // Enviar una respuesta de error 400 (Bad Request)
                    fclose(res_file);
                    response(new_socket, 400, "Bad Request", "text/html", "<h1>400 Hiciste mal el request pa :(</h1>");
                }
                
            }

        } else{

            printf("El método no es GET\n");
            FILE * res_file = fopen(full_path, "r");
            // Validar que el archivo de respuesta exista
            if (res_file == NULL){
                // Enviar una respuesta de error 404 porque no se encontró el archivo de respuesta
                response(new_socket, 404, "Not Found", "text/html", "<h1>404 No se encontró lo que está buscando pa :(</h1>");
            } else if(strcmp(method, "HEAD") == 0){
                
                // El método es HEAD y el archivo de respuesta existe
                printf("El método es HEAD y si se encontró :D\n");
                fclose(res_file);
                // Enviar una respuesta de éxito 200
                response(new_socket, 200, "OK", "text/html", "");
            
            } else if (strcmp(method, "POST") == 0){

                // El método es POST y el archivo de respuesta existe
                int length = 0;
                // Obtener el valor de Content-Length
                char * length_header = strstr(buffer, "Content-Length: ");
                // Validar que se haya encontrado el header Content-Length
                if (length_header != NULL){
            
                    length = atoi(length_header + strlen("Content-Length: "));
                }

                // Leer el body del request 
                char * body = malloc(length + 1);
                int body_len = 0;
                // Leer el body del request mientras no se haya leído todo
                while (body_len < length){
                    // Leer el body del request
                    int read_len = read(new_socket, body + body_len, length - body_len);
                    // Validar que se hayan leído datos
                    if (read_len < 0){
                        perror("Error leyendo el body del request :(");
                        exit(EXIT_FAILURE);
                    }
                    // Actualizar la longitud del body
                    body_len += read_len;
                }           
                // Agregar el caracter nulo al final del body
                body[length] = '\0';
                // Imprimir el body del request
                printf("Body: %s\n", body);
                // Enviar una respuesta de éxito 200
                response(new_socket, 200, "OK", "text/html", "<h1>200 Todo bien pa :D</h1>");
                // Liberar la memoria del body
                free(body);

            } else{
                // Enviar una respuesta de error 400 (Bad Request)
                fclose(res_file);
                response(new_socket, 400, "Bad Request", "text/html", "<h1>400 Hiciste mal el request pa :(</h1>");
            }

        }
        // Cerramos el socket del cliente 
        close(new_socket);
    }

    // retornamos 0 porque todo salió bien 
    return 0;

}



