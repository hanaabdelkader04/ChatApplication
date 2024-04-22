#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//function to relay messages between clients
void relayMessages(SOCKET from, SOCKET to, const string& fromLabel, ofstream& logFile) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(from, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << fromLabel << " says: " << buffer << endl;
            logFile << fromLabel << ": " << buffer << endl;

            //sends message to the other client immediately
            send(to, buffer, bytesReceived, 0);
        } else {
            if (bytesReceived == 0) {
                cout << fromLabel << " disconnected." << endl;
            } else {
                cerr << "recv failed with error: " << WSAGetLastError() << endl;
            }
            return;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket1, clientSocket2;
    sockaddr_in serverAddr;
    ofstream chatHistory("chat_history.txt", ios::out | ios::app);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed: " << WSAGetLastError() << endl;
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 2) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Waiting for clients to connect..." << endl;

    clientSocket1 = accept(serverSocket, NULL, NULL);
    if (clientSocket1 == INVALID_SOCKET) {
        cerr << "Accept failed for client 1: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Client 1 connected." << endl;

    clientSocket2 = accept(serverSocket, NULL, NULL);
    if (clientSocket2 == INVALID_SOCKET) {
        cerr << "Accept failed for client 2: " << WSAGetLastError() << endl;
        closesocket(clientSocket1);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Client 2 connected. Starting communication..." << endl;

    //threads to handle communication
    thread client1ToClient2(relayMessages, clientSocket1, clientSocket2, "Client 1", ref(chatHistory));
    thread client2ToClient1(relayMessages, clientSocket2, clientSocket1, "Client 2", ref(chatHistory));

    // wait for both threads to finish
    client1ToClient2.join();
    client2ToClient1.join();

    // clean up
    chatHistory.close();
    closesocket(clientSocket1);
    closesocket(clientSocket2);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
