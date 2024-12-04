#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

class Client {
private:
    int socket_fd;
    sockaddr_in server_address;

    void receiveMessages() {
        char buffer[BUFFER_SIZE];
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(socket_fd, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                std::cout << "Desconectado del servidor.\n";
                break;
            }
            std::cout << buffer << std::endl;
        }
    }

public:
    Client() : socket_fd(-1) {
        memset(&server_address, 0, sizeof(server_address));
    }

    ~Client() {
        if (socket_fd != -1) {
            close(socket_fd);
        }
    }

    bool connectToServer(const std::string& server_ip) {
        // Crear socket
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error al crear el socket");
            return false;
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        // Convertir la dirección IP
        if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0) {
            perror("Dirección inválida o no soportada");
            return false;
        }

        // Conectar al servidor
        if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            perror("Error al conectar con el servidor");
            return false;
        }

        std::cout << "Conectado al servidor.\n";
        return true;
    }

    void start() {
        // Enviar el nombre del cliente
        std::string client_name;
        std::cout << "Introduce tu nombre: ";
        std::getline(std::cin, client_name);
        send(socket_fd, client_name.c_str(), client_name.size(), 0);

        // Crear un hilo para recibir mensajes
        std::thread receiver_thread(&Client::receiveMessages, this);
        receiver_thread.detach();

        // Manejar el envío de mensajes
        std::string message;
        while (true) {
            std::getline(std::cin, message);
            if (message == "/salir") {
                break;
            }
            send(socket_fd, message.c_str(), message.size(), 0);
        }

        std::cout << "Desconectado del servidor.\n";
    }
};

int main() {
    Client client;
    if (client.connectToServer("127.0.0.1")) {
        client.start();
    }
    return 0;
}

