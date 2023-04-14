#include "common.h"
#include <strings.h>

#define MAX_POST_DATA 1024
char postData[MAX_POST_DATA];

int main(int argc, char **argv){

    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    uint8_t buff[MAXLINE+1];
    uint8_t recvline[MAXLINE+1];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Error creando el socket!");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT); //Puerto 18000

    if ((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0)
        err_n_die("Error en el bind!");

    if ((listen(listenfd, 10)) < 0)
        err_n_die("Error en el listen!");

    for (;;) {
        struct sockaddr_in addr;
        socklen_t addr_len;

        //Esperando por una conexión
        printf("Esperando por una conexión en el puerto %d\n", SERVER_PORT);
        fflush(stdout);

        connfd = accept(listenfd, (SA *) NULL, NULL);

        memset(recvline, 0, MAXLINE);

        //Leemos el mensaje del cliente
        while ((n = read(connfd, recvline, MAXLINE-1)) > 0)
        {
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n),recvline);

            //Detectamos el final del mensaje
            if (recvline[n-1] == '\n')
                break;
        }

        memset(recvline, 0, MAXLINE);

        if (n < 0)
            err_n_die("Error al leer");

        //Parseamos la solicitud del cliente
        char *method = strtok((char*)recvline, " \t\r\n");
        char *uri = strtok(NULL, " \t");
        char *version = strtok(NULL, " \t\r\n");
        char *headers = strtok(NULL, "\r\n");
        int contentLength = 0;
        char *contentType = NULL;

        while (headers != NULL) {
            if (strncasecmp(headers, "Content-Length:", 15) == 0) {
                contentLength = atoi(headers+15);
            } else if (strncasecmp(headers, "Content-Type:", 13) == 0) {
                contentType = headers+13;
            }

            headers = strtok(NULL, "\r\n");
        }

        //Procesamos la solicitud del cliente
        if (strcasecmp(method, "GET") == 0) {
            printf("Solicitud GET para %s\n", uri);
            snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\nHello");
            write(connfd, (char*)buff, strlen((char*)buff));
        } else if (strcasecmp(method, "HEAD") == 0) {
            printf("Solicitud HEAD para %s\n", uri);
            snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\n");
            write(connfd, (char*)buff, strlen((char*)buff));
        } else if (strcasecmp(method, "POST") == 0) {
            printf("Solicitud POST para %s con tipo de contenido %s y longitud de contenido %d\n", uri, contentType, contentLength);

            if (contentLength > MAX_POST_DATA) {
                snprintf((char*)buff, sizeof(buff), "HTTP/1.1 400 Bad Request\r\n\r\n");
                write(connfd, (char*)buff, strlen((char*)buff));
                err_n_die("Error: Se excedió el tamaño máximo permitido para los datos POST.");
            }

            /* Lee los datos POST */
            int numBytes = 0;
            while (numBytes < contentLength) {
                n = read(connfd, &postData[numBytes], contentLength - numBytes);
                if (n < 1) {
                    snprintf((char*)buff, sizeof(buff), "HTTP/1.1 400 Bad Request\r\n\r\n");
                    write(connfd, (char*)buff, strlen((char*)buff));
                    err_n_die("Error al leer los datos POST.");
                }
                numBytes += n;
            }

            /* Agrega el terminador de cadena al final de los datos POST */
            postData[contentLength] = '\0';

            /* Envía la respuesta */
            snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\nHello");
            write(connfd, (char*)buff, strlen((char*)buff));

            /* Imprime los datos POST */
            fprintf(stdout, "Datos POST recibidos:\n%s\n", postData);

            close(connfd);

        } else {
            snprintf((char*)buff, sizeof(buff), "HTTP/1.1 404 Not Found\r\n\r\n");
            write(connfd, (char*)buff, strlen((char*)buff));
            err_n_die("Error: Método HTTP no soportado.");
        }}}

