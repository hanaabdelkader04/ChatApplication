#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// Caesar cipher encryption
string caesarEncrypt(const string& text, int shift) {
    string result;
    for (char c : text) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            result += static_cast<char>((c - base + shift) % 26 + base);
        } else {
            result += c;
        }
    }
    return result;
}

// Caesar cipher decryption
string caesarDecrypt(const string& text, int shift) {
    return caesarEncrypt(text, 26 - shift);
}

// Function to handle messages from the server
void receiveMessages(SOCKET clientSocket) {
    const int key = 13;  // Using a fixed key for simplicity
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string decryptedMessage = caesarDecrypt(buffer, key);
            cout << "\nServer: " << decryptedMessage << "\n";
            cout << "Enter message: ";
            fflush(stdout);
        } else if (bytesReceived == 0) {
            cout << "\nServer disconnected." << endl;
            break;
        } else {
            cerr << "\nReceive failed with error: " << WSAGetLastError() << endl;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    const int key = 13;  // Fixed Caesar cipher key
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed: " << result << endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Connection failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected successfully. Enter your messages. Type 'exit' to quit:" << endl;

    // Start the thread for receiving messages from the server
    thread receiverThread(receiveMessages, clientSocket);

    string input;
    while (true) {
        cout << "Enter message: ";
        getline(cin, input);
        if (input == "exit") {
            send(clientSocket, input.c_str(), input.length(), 0); // Notify server about the exit
            break;
        }
        string encryptedMessage = caesarEncrypt(input, key);
        if (send(clientSocket, encryptedMessage.c_str(), encryptedMessage.length(), 0) == SOCKET_ERROR) {
            cout << "Send failed with error: " << WSAGetLastError() << endl;
            break;
        }
    }

    // Clean up
    receiverThread.join(); // Ensure the receive thread has finished
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
