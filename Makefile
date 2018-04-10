.PHONY: build

build:
	xcodebuild -target osx-motion-tracking-cpp -configuration Debug -project osx-motion-tracking-cpp.xcodeproj

run: build
	./build/Debug/osx-motion-tracking-cpp
