Video Event Clipping System

Overview
========
This project implements a simple real-time video event clipping system in C++ using shared memory, sockets, multithreading, and OpenCV.
The system contains four applications:
1.Video Processor
  Reads frames from a video file
  Shares frames through shared memory
  Maintains source FPS timing
2.Video Clipper
  Reads frames from shared memory
  Maintains a circular buffer for the previous 9 seconds
  Receives event notifications over socket
  Generates a 15-second clip:
  9 seconds before event
  6 seconds after event
  Saves clip as .mp4
3.Event Notifier
  User interactive application
  Sends event notification when user presses e
4.CPU Watcher
  Monitors system memory usage
  Monitors Video Clipper process usage

Requirements
============
•Linux
•C++17
•OpenCV
•CMake
•POSIX shared memory

Design Highlights
==================
•Shared memory based IPC
•Mutex protected shared memory synchronization
•Circular frame buffer using deque
•Multi-threaded event processing
•Graceful shutdown using SIGINT

References
===========
•LearnOpenCV - Reading and Writing Videos using OpenCV
•OpenCV Official Documentation
•ChatGPT

Notes
=======
•Sometimes process synchronization may fail and generate unusually large output files.
•CPU usage printing is incomplete due to time limitations.
•Shared memory synchronization uses pthread_mutex.

Compile
========

mkdir build
cd build

cmake ..
make

cd -


Run
===

Open four terminals and execute:

Terminal 1
./build/video_processor
Terminal 2
./build/video_clipper
Terminal 3
./build/event_notifier
Terminal 4
./build/CPU_watcher
Trigger Event

Inside event_notifier:

Press e + ENTER

Each event generates an independent video clip.


=======Output=====

Generated clips are saved as:

<timestamp>.mp4

Example:

1715578210.mp4
