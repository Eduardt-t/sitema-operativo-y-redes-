Integrantes: Catalina Figueroa, Eduardo Toledo y Simone Urrutia.

Este programa implementa un sistema de chat, el cual permite enviar mensajes públicos y privados.

El código utiliza hilos por lo que el siguiente comando es un ejemplo de como compilar y ejecutar el programa:

Para el Servidor:
g++ Servidor.cpp -pthread -o servidor
./servidor

Para el Cliente:
g++ Cliente.cpp -pthread -o cliente
./cliente

Para poder enviar un mensaje privado se debe utilizar el comando:
/privado <nombre_cliente> <mensaje>

Los mensajes tanto públicos como privados pueden tener un largo de 1024 caracteres.

Para salir el cliente debe utilizar el comando:
/salir