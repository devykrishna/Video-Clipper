#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

using namespace std;

// Atomic flag to signal the loop to stop
atomic<bool> keep_running(true);

// Signal handler function called when Ctrl+C is pressed
void signalHandler(int sig_num)
{  
    if (sig_num == SIGINT) {
        keep_running = false;
    }
}

void sendEvent()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    // Convert IPv4 address from text to binary
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Connection failed\n";
        return;
    }

    string msg = "EVENT";

    send(sock, msg.c_str(), msg.size(), 0);

    close(sock);
}

int main()
{
     // Register the handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);

    cout
        << "Press e + ENTER to send event\n";

    while (keep_running) {

        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int rv = select(STDIN_FILENO + 1,
                        &set,
                        NULL,
                        NULL,
                        &timeout);
// rv = The number of ready descriptors, 0 on timeout, or -1 on error
        if (rv > 0) {
            char c;
            cin >> c;
            if (c == 'e') {
                thread(sendEvent).detach();
                cout << "Event sent\n";
            }
        }
    }
    cout << "\nStop signal received. Cleaning up and exiting..." << endl;
    return 0;
}