#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

class Server {
private:
    int server_fd;
    sockaddr_in address;
    int opt;
    std::unordered_map<int, std::string> clients; // Mapa de socket -> nombre
    std::mutex clients_mutex;

    void broadcastMessage(const std::string& message, int sender_socket) {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& [socket, name] : clients) {
            if (socket != sender_socket) {
                send(socket, message.c_str(), message.size(), 0);
            }
        }
    }

    void privateMessage(const std::string& recipient_name, const std::string& message, int sender_socket) {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& [socket, name] : clients) {
            if (name == recipient_name) {
                send(socket, message.c_str(), message.size(), 0);
                return;
            }
        }
        // Si no se encuentra el destinatario
        std::string error_msg = "Error: destinatario no encontrado\n";
        send(sender_socket, error_msg.c_str(), error_msg.size(), 0);
    }

    void handleClient(int client_socket) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        // Recibir el nombre del cliente
        if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
            close(client_socket);
            return;
        }
        std::string client_name = buffer;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[client_socket] = client_name;
        }
        std::cout << client_name << " se ha conectado.\n";

        // Manejo de mensajes
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                break;
            }

            std::string message = buffer;

            // Mensaje privado (formato: /privado <nombre> <mensaje>)
            if (message.rfind("/privado ", 0) == 0) {
                size_t space_pos = message.find(' ', 9);
                if (space_pos != std::string::npos) {
                    std::string recipient_name = message.substr(9, space_pos - 9);
                    std::string private_message = client_name + " (privado): " + message.substr(space_pos + 1);
                    privateMessage(recipient_name, private_message, client_socket);
                }
            } else { // Mensaje público
                std::string public_message = client_name + ": " + message;
                broadcastMessage(public_message, client_socket);
            }
        }

        // Desconexión del cliente
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::cout << clients[client_socket] << " se ha desconectado.\n";
            clients.erase(client_socket);
        }
        close(client_socket);
    }

public:
    Server() : server_fd(0), opt(1) {
        memset(&address, 0, sizeof(address));
    }

    ~Server() {
        if (server_fd != 0) {
            close(server_fd);
        }
    }

    bool start() {
        // Crear socket del servidor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Error al crear el socket");
            return false;
        }

        // Configuración del socket
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("Error en setsockopt");
            close(server_fd);
            return false;
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Vincular el socket
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("Error en bind");
            close(server_fd);
            return false;
        }

        // Escuchar conexiones entrantes
        if (listen(server_fd, 10) < 0) {
            perror("Error en listen");
            close(server_fd);
            return false;
        }

        std::cout << "Servidor escuchando en el puerto " << PORT << "\n";

        while (true) {
            int new_socket;
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
                perror("Error en accept");
                continue;
            }

            std::thread(&Server::handleClient, this, new_socket).detach();
        }
        return true;
    }
};

int main() {
    Server server;
    if (server.start()) {
        std::cout << "Servidor iniciado correctamente.\n";
    } else {
        std::cerr << "Error al iniciar el servidor.\n";
    }
    return 0;
}

