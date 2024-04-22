#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddress;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return 1;
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Setup the server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); // Server port
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr); // Server IP address

    // Connect to the server
    cout << "Connecting to server..." << endl;
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Connection failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server. Enter a message: " << endl;
    
    // Get message from the user using cin.getline
    char message[1024];
    cin.getline(message, 1024);

    // Send data to the server
    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent == SOCKET_ERROR) {
        cout << "Failed to send message with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Receive response from the server
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        cout << "Server says: " << string(buffer, 0, bytesReceived) << endl;
    } else if (bytesReceived == 0) {
        cout << "Connection closed by server." << endl;
    } else {
        cout << "recv failed with error: " << WSAGetLastError() << endl;
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
