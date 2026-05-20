#include "common/sharedFrame.hpp"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

#include <deque>
#include <mutex>
#include <vector>
#include <chrono>
#include <fstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

using namespace std;
using namespace cv;

// Atomic flag to signal the loop to stop
atomic<bool> keep_running(true);
// Signal handler function called when Ctrl+C is pressed
void signalHandler(int sig_num)
{  
    if (sig_num == SIGINT) {
        keep_running = false;
    }
}

struct FramePacket {
    Mat frame;
    uint64_t timestamp;
};

deque<FramePacket> frameBuffer;

mutex bufferMutex;


double g_fps = 0.0;

// Returns current timestamp in milliseconds
uint64_t nowMs()
{
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}


// Saves frames into mp4 video file
void saveClip(std::vector<FramePacket> frames)
{
    // No frames to save
    if (frames.empty())
        return;

    // Create filename using current timestamp
    string filename = to_string(nowMs()) + ".mp4";
    Size frameSize(frames[0].frame.cols,frames[0].frame.rows);
   //Size frameSize(854,480);
    int fcc = VideoWriter::fourcc('m', 'p', '4', 'v'); // Codec for .mp4
    // Create OpenCV video writer
    VideoWriter writer(filename,fcc, g_fps,frameSize);

    // Write all frames into video
    for (auto& f : frames)
        writer.write(f.frame);

    writer.release();

    cout << "Saved clip: " << filename << " fps " << g_fps<< "\n";
}
 //uint64_t lastFrame = 0;
// Handles one event trigger
// Creates:
// 9 seconds BEFORE event
// 6 seconds AFTER event
void handleEvent()
{
    cout << "Event received. Creating clip...\n";

    // Final video frames
    vector<FramePacket> clipFrames;
    SharedFrame shared(false);
    pthread_mutex_lock(&shared.sharedData->ShMmutex);
    // =========================
    // Copy previous 9 seconds
    // =========================
    {
        // Lock buffer while copying
        lock_guard<mutex> lock(bufferMutex);

        // Copy all frames from circular buffer to vector
        for (auto& f : frameBuffer)
        {
            clipFrames.push_back({
                f.frame.clone(),
                f.timestamp
            });
        }
        //cout << "last frame number" << lastFrame<< endl;
    }
    // =========================
    // Capture next 6 seconds
    // =========================

    // Number of frames needed
    int futureFrames = static_cast<int>(6 * g_fps);
    int maxFrames = (int)clipFrames.size() + futureFrames;

    //cout << "fps" << g_fps << endl;
    //cout << "future frames" << futureFrames << endl;
    // Access shared memory
    
    uint64_t frameNo;
    uint64_t lastFrameNumber = 0;
    int delayMs = 1000 / g_fps;

    while ((int)clipFrames.size() < maxFrames) {
                    
    //for (int i = 0; i < futureFrames; i++) {
        Mat frame;

        // Read latest frame from shared memory
        if (shared.readFrame(frame, frameNo, g_fps))
        {
            if(frameNo != lastFrameNumber)
            {
            clipFrames.push_back({ frame.clone(), nowMs() });
            lastFrameNumber = frameNo;
            }
            //cout << "current frame number" << frameNo<< endl;
        }
        pthread_mutex_unlock(&shared.sharedData->ShMmutex);
        // Wait according to FPS
        //waitKey(delayMs);
        this_thread::sleep_for(chrono::milliseconds(1));
        
    }

    // Save final clip
    saveClip(clipFrames);
}

// Thread continuously reading frames
// from shared memory and keep latest 9s video fraes only
void frameReaderThread()
{
    // Open existing shared memory
    SharedFrame shared(false);
    uint64_t lastFrame = 0;
   
    while (keep_running)
    {
        Mat frame;
        uint64_t frameNo;

        // Read latest frame
        if (shared.readFrame(frame, frameNo, g_fps))
        {
            // Avoid processing duplicate frames
            if (frameNo != lastFrame)
            {
                lastFrame = frameNo;
                {
                    // Lock circular buffer
                    lock_guard<mutex> lock(bufferMutex);

                    // Store frame in buffer
                    frameBuffer.push_back({frame.clone(), nowMs()});

                    // Maximum frames for 9 seconds
                    int maxFrames = static_cast<int>(9 * g_fps);
                    //cout << " read fps = "<< g_fps << endl;
                    // Remove oldest frames
                    // to maintain fixed buffer size
                    while ((int)frameBuffer.size() > maxFrames)
                    {
                        frameBuffer.pop_front();
                    }
                }
            }
        }
        // Small delay to reduce CPU usage
        this_thread::sleep_for(chrono::milliseconds(1));
    }
   // cout << "Exit frame reader thread" << endl;
}

// Thread waiting for EVENT messages
// over TCP socket
void socketServerThread()
{
    // Create TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address;
    address.sin_family = AF_INET;
    // Accept connection from any IP
    address.sin_addr.s_addr = INADDR_ANY;
    // Port number
    address.sin_port = htons(8080);

    // Bind socket to port
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    // Start listening
    listen(server_fd, 5);

    cout << "Waiting for event notifications...\n";
    
    while (keep_running)
    {
        socklen_t addrlen = sizeof(address);
        int client_socket;

        fd_set set;
        FD_ZERO(&set);
        FD_SET(server_fd, &set);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int rv = select(server_fd + 1, &set, NULL, NULL, &timeout);
        if (rv > 0)
        {
            // Accept incoming client connection
            client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

            char buffer[1024] = {0};
            // Read message from client
            read(client_socket, buffer, 1024);

            string msg(buffer);

            // Check if message is EVENT
            if (msg == "EVENT")
            {
                cout << "EVENT trigger received\n";

            // Create separate thread
            // so multiple events can be processed
            // simultaneously
                thread(handleEvent).detach();
            }
            close(client_socket);
        }
    }

    close(server_fd);
    // cout << "Exit socket thread" << endl;
}

int main()
{
    signal(SIGINT, signalHandler);
    
    //save pid
    // Get the current PID
    pid_t pid = getpid();
   // cout << "Current PID: " << pid << endl;

    // Save PID to a file
    ofstream pidFile("video_clipper.pid");
    if (pidFile.is_open()) {
        pidFile << pid;
        pidFile.close();
       // cout << "PID saved to video_clipper.pid" << endl;
    } else {
        cerr << "Unable to open file" << endl;
    }

    thread t1(frameReaderThread);// latest 9s video in the circular buffer

    thread t2(socketServerThread); // receive event

    t1.join();
    //cout << "T1 joined" << endl;
    t2.join();

    cout << "Video Clipper stopped\n";

    return 0;
}
