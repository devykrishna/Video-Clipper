// Include Libraries
#include <opencv2/opencv.hpp>
#include "common/sharedFrame.hpp"
#include <iostream>
#include <atomic>
#include <csignal>
#include <thread>
#include <pthread.h>

// Namespace to nullify use of cv::function(); syntax
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

int main()
{
	// Register the handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);
	double fps = 0.0;
	// 1. Open the video source
	// initialize a video capture object
	VideoCapture vid_capture("Resources/traffic.mp4");

    // Create shared memory
    // true means create shared memory region
    SharedFrame shared(true);
	pthread_mutexattr_t attr;
	//initialized only once by the creator process using PTHREAD_PROCESS_SHARED attribute 
	//to enable synchronization across multiple processes
	pthread_mutexattr_init(&attr);

	//IMPORTANT: allow inter-process sharing
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

	// initialize mutex inside shared memory
	pthread_mutex_init(&shared.sharedData->ShMmutex, &attr);

	// Print error message if the stream is invalid
	if (!vid_capture.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
	}

	else
	{
		// Obtain fps and frame count by get() method and print
		// You can replace 5 with CAP_PROP_FPS as well, they are enumerations
		fps = vid_capture.get(5);
		cout << "Frames per second :" << fps;

		// Obtain frame_count using opencv built in frame count reading method
		// You can replace 7 with CAP_PROP_FRAME_COUNT as well, they are enumerations
		int frame_count = vid_capture.get(7);
		cout << "  Frame count :" << frame_count << endl;
		int frame_width = static_cast<int>(vid_capture.get(3));
		cout << "width :" << frame_width << endl;
		int frame_height = static_cast<int>(vid_capture.get(4));
		cout << "height :" << frame_height << endl;
	}
	// Initialise frame matrix
	Mat frame;
    int frameCount = 0;

	// 2. Loop through each frame
	// Read the frames to the last frame
	while (vid_capture.isOpened() && keep_running)
	{

	    // Initialize a boolean to check if frames are there or not
		bool isSuccess = vid_capture.read(frame);

		// If frames are present, save it
		if(isSuccess == true)
		{
		// Capture frame-by-frame
        vid_capture >> frame;

        // If the frame is empty, the video has ended
        if (frame.empty()) break;
        // Write frame into shared memory
		pthread_mutex_lock(&shared.sharedData->ShMmutex);
        shared.writeFrame(frame, frameCount, fps);
		pthread_mutex_unlock(&shared.sharedData->ShMmutex);
 	/*	//3. Save the frame separately
        // Generate a filename: e.g., frame_0.jpg, frame_1.jpg...
        string filename = "Resources/frame_" + to_string(frameCount) + ".jpg";

		// Use imwrite to save the image
        imwrite(filename, frame);*/
		frameCount++;

		}

		// If frames are not there, close it
		if (isSuccess == false)
		{
			cout << "File is ended" << endl;
			break;
		}
		
	 this_thread::sleep_for(chrono::milliseconds(1));
	}
	// Release the video capture object
	vid_capture.release();
	cout << "Successfully saved " << frameCount << " frames." << endl;
	return 0;
}
