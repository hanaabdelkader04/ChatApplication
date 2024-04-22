#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//function to handle messages from the server
void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "\nServer: " << string(buffer) << "\n";
            cout << "Enter message: ";
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

    //start the thread for receiving messages from the server
    thread receiverThread(receiveMessages, clientSocket);

    string input;
    while (true) {
        cout << "Enter message: ";
        getline(cin, input);
        if (input == "exit") {
            break;
        }

        if (send(clientSocket, input.c_str(), input.length(), 0) == SOCKET_ERROR) {
            cout << "Send failed with error: " << WSAGetLastError() << endl;
            break;
        }
    }

    //clean up
    receiverThread.join();
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
