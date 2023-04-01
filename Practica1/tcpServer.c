#include "common.h"
#include <strings.h>

int main(int argc, char **argv)
{
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
    
    if ((listen(listenfd, 10 )) < 0)
        err_n_die("Error en el listen!");
    
    for ( ; ; ) {
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

    }

    if (n < 0)
        err_n_die("Error al leer");
    
    //Enviamos la respuesta
    snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\nHello");

    write(connfd, (char*)buff, strlen((char*)buff));
    close(connfd);

}
void err_n_die(const char *fmt, ...)
{
    int errno_save;
    va_list     ap;

    errno_save = errno;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if(errno_save != 0)
    {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);

    // Esta es la parte n_die que hace que el programa se cierre con un error
    exit(1);
}
char *bin2hex(const unsigned char *input, size_t len)
{
    char *result;
    char *hexits = "0123456789ABCDEF";

    if (input == NULL || len <= 0)
        return NULL;
    
    int resultlength = (len*3) +1;

    result = malloc(resultlength);
    bzero(result, resultlength);

    for (int i = 0; i < len; i++)
    {
        result[i*3] = hexits[input[i] >> 4];
        result[(i*3)+1] = hexits[input[i] & 0x0F];
        result[(i*3)+2] = ' ';//Para poderlo leer más facil
    }

    return result;
}
