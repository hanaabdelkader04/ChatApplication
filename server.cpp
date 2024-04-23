#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// Caesar cipher encryption
string caesarEncrypt(const string& text, int shift) {
    string encrypted = "";
    for (char c : text) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            encrypted += static_cast<char>((c - base + shift) % 26 + base);
        } else {
            encrypted += c;
        }
    }
    return encrypted;
}

// Caesar cipher decryption
string caesarDecrypt(const string& text, int shift) {
    return caesarEncrypt(text, 26 - shift);
}

// Function to relay messages between clients
void relayMessages(SOCKET from, SOCKET to, const string& fromLabel, ofstream& logFile, int key) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(from, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string message = caesarDecrypt(string(buffer), key);
            cout << fromLabel << " says: " << message << endl;
            
            string encryptedLog = caesarEncrypt(message, key);
            logFile << fromLabel << ": " << encryptedLog << endl;
            
            string encryptedMessage = caesarEncrypt(message, key);
            send(to, encryptedMessage.c_str(), encryptedMessage.length(), 0);
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed: " << WSAGetLastError() << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr = {};
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

    SOCKET clientSocket1 = accept(serverSocket, NULL, NULL);
    if (clientSocket1 == INVALID_SOCKET) {
        cerr << "Accept failed for client 1: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Client 1 connected." << endl;

    SOCKET clientSocket2 = accept(serverSocket, NULL, NULL);
    if (clientSocket2 == INVALID_SOCKET) {
        cerr << "Accept failed for client 2: " << WSAGetLastError() << endl;
        closesocket(clientSocket1);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "Client 2 connected. Starting communication..." << endl;

    srand(time(nullptr));
    int encryptionKey = rand() % 25 + 1; // Random key between 1 and 25

    ofstream chatHistory("chat_history.txt", ios::out | ios::app);

    thread client1ToClient2(relayMessages, clientSocket1, clientSocket2, "Client 1", ref(chatHistory), encryptionKey);
    thread client2ToClient1(relayMessages, clientSocket2, clientSocket1, "Client 2", ref(chatHistory), encryptionKey);

    client1ToClient2.join();
    client2ToClient1.join();

    chatHistory.close();
    closesocket(clientSocket1);
    closesocket(clientSocket2);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}