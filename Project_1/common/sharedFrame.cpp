#include "sharedFrame.hpp"

SharedFrame::SharedFrame(bool create)
{
    // Create shared memory
    if (create)
    {
        shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

        // Set memory size
        ftruncate(shmFd, sizeof(SharedData));
    }
    else
    {
        // Open existing shared memory
        shmFd = shm_open(SHM_NAME, O_RDWR, 0666);
    }

    // Map shared memory into process memory
    sharedData = (SharedData*) mmap(NULL, sizeof(SharedData),PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0 );

    if (sharedData == MAP_FAILED)
    {
        std::cout << "Shared memory mapping failed\n";
    }
}

SharedFrame::~SharedFrame()
{
    // Remove mapping
    munmap(sharedData, sizeof(SharedData));
    // Close shared memory
    close(shmFd);
}

void SharedFrame::writeFrame(const cv::Mat& frame,
                             uint64_t frameNumber, double fps)
{
  
    // Save frame info
    sharedData->width = frame.cols;
    sharedData->height = frame.rows;
    sharedData->channels = frame.channels();
    sharedData->frameNumber = frameNumber;
    sharedData->fps = fps;

    // Calculate image size
    int imageSize = frame.total() * frame.elemSize();

    // Copy image bytes
    memcpy(sharedData->frameData, frame.data, imageSize);
  
}

bool SharedFrame::readFrame(cv::Mat& frame,
                            uint64_t& frameNumber, double& fps)
{
    // Check valid frame
    if (sharedData->width <= 0 ||
        sharedData->height <= 0)
    {
        return false;
    }
    // Create OpenCV image using shared memory
    //CV_8UC3 - 8-bit Unsigned, 3 Channels (BGR).
    cv::Mat temp_frame(
        sharedData->height,
        sharedData->width,
        CV_8UC3,
        sharedData->frameData
    );

    // Clone image
    // Important because shared memory changes continuously
    frame = temp_frame.clone();
    frameNumber = sharedData->frameNumber;
    fps = sharedData->fps;
    return true;
}
