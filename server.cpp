#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddress;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Set up the sockaddr structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == SOCKET_ERROR) { // Listen backlog set to 1
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Accept a connection
    cout << "Waiting for a client to connect..." << endl;
    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Accept failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Client connected." << endl;

    // Receive data from the client
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        cout << "Received message: " << string(buffer, 0, bytesReceived) << endl;
    } else if (bytesReceived == 0) {
        cout << "Connection closing..." << endl;
    } else {
        cout << "recv failed with error: " << WSAGetLastError() << endl;
    }

    // Send a response to the client
    string response = "Message received.";
    send(clientSocket, response.c_str(), response.length(), 0);

    // Clean up
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
