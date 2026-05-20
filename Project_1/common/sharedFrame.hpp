#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>


#define SHM_NAME "/video_shared_memory"

// Max frame size:
// 1920 x 1080 x 3 channels (RGB)
#define MAX_FRAME_SIZE (1920 * 1080 * 3)


struct SharedData
{
    int width;
    int height;
    int channels;
    double fps;
    uint64_t frameNumber;
    unsigned char frameData[MAX_FRAME_SIZE];// Raw image bytes
    pthread_mutex_t ShMmutex;
};

class SharedFrame
{
public:

    SharedFrame(bool create);
    ~SharedFrame();
    void writeFrame(const cv::Mat& frame, uint64_t frameNumber, double fps);
    bool readFrame(cv::Mat& frame, uint64_t& frameNumber, double& fps);
   SharedData* sharedData;
private:

    int shmFd;
    
};