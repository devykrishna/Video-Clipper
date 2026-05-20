# Video-Clipper
======Compile======

mkdir build
cd build

cmake ..
make

cd -


=====Run===========

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