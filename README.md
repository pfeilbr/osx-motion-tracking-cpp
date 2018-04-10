# osx-motion-tracking-cpp

macOS C++ app that manipulates a 3D object based on motion tracking of an object.  Uses [OpenCV](https://opencv.org/) for object tracking.

## Building and Running from Command Line

```sh
# compile from command line
xcodebuild -target osx-motion-tracking-cpp -configuration Debug -project osx-motion-tracking-cpp.xcodeproj

# run from command line
./build/Debug/osx-motion-tracking-cpp
```