# Web Server en C
Este es un servidor web básico implementado en C creado por Tomás Duque y David Ruiz que puede manejar solicitudes GET, HEAD, POST para archivos HTML. El servidor web utiliza sockets para conectarse a un puerto especificado y esperar conexiones entrantes. El archivo solicitado se busca en una carpeta especificada y se envía al cliente en una respuesta HTTP.

## Instalación
Este servidor web se puede compilar y ejecutar en sistemas operativos basados en Unix (Linux, macOS, etc.). Se requiere el compilador GCC para compilar el código fuente. Para compilar el servidor, abra una terminal y escriba el siguiente comando:

```
 gcc HTTP_server.c -o HTTP_server

```

Esto generará un archivo ejecutable llamado `HTTP_server`. Luego, ejecute el servidor escribiendo:

```
 ./HTTP_server <puerto> <ruta_archivo_log> <ruta_carpeta_archivos>

```

Reemplace `<puerto>`, `<ruta_archivo_log>` y `<ruta_carpeta_archivos>` con los valores apropiados. El puerto es el número de puerto que el servidor debe usar para escuchar conexiones entrantes. El `ruta_archivo_log` es el archivo donde se registran las solicitudes entrantes del cliente. La `ruta_carpeta_archivos` es la ruta de la carpeta que contiene los archivos HTML que se pueden solicitar.

## Uso

Una vez que se ha ejecutado el servidor, puede solicitar un archivo HTML escribiendo la siguiente dirección en un navegador web:

```
 http://localhost:<puerto>/<nombre_archivo>.html

```

Reemplace `<puerto>` con el número de puerto que se usó para iniciar el servidor y `<nombre_archivo>` con el nombre del archivo HTML que desea solicitar. El servidor buscará el archivo en la carpeta especificada y lo enviará al navegador web en una respuesta HTTP. Si el archivo no se encuentra, se enviará una respuesta de error 404 al cliente.

## Estructura del código fuente

El código fuente del servidor se divide en varias secciones:

1. Declaraciones e inclusión de bibliotecas
2. Función de registro de mensajes
3. Función de respuesta HTTP
4. Función principal `main`

En la sección de declaraciones e inclusión de bibliotecas, se incluyen las bibliotecas necesarias para crear y manipular sockets, leer y escribir archivos, y manejar cadenas de caracteres. También se define un tamaño de búfer para almacenar los datos recibidos del cliente.

La función de registro de mensajes se utiliza para escribir solicitudes entrantes del cliente en un archivo de registro especificado. Si el archivo de registro no se puede abrir, el servidor termina con un error.

La función de respuesta HTTP se utiliza para enviar una respuesta HTTP al cliente después de recibir una solicitud. Toma varios argumentos, como el código de estado, el mensaje de estado, el contenido y el cuerpo del mensaje.

La función principal `main` es el punto de entrada del servidor. Toma los argumentos de entrada, valida que sean correctos y establece la conexión en el puerto especificado. Luego, acepta conexiones entrantes y maneja cada solicitud en una conexión separada.
