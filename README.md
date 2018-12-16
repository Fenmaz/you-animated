# You, Animated!

![alt text](https://github.com/Fenmaz/you-animated/blob/bones/resources/Screen%20Shot%202018-11-27%20at%208.49.22%20PM.png =250 "Rigged Model")


Project to animate humans as 3D models from 2D video input.

## Setup on MacOS
### Basic requirements
1. Install [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)
2. Install [CMake](https://cmake.org/download/)
3. To install command line tools for CMake, run
```
sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
```
and input user's password if asked.
4. Install [XCode](https://developer.apple.com/xcode/) and the [command line tools](https://www.moncefbelyamani.com/how-to-install-xcode-homebrew-git-rvm-ruby-on-mac/#laptop-script).

### Required libraries
1. Install [MinVR](https://github.com/mac-comp465-f18/minvr).
2. Install [basicgraphics](https://github.com/mac-comp465-f18/basicgraphics) following these steps from your development directory:
  ```
  git clone https://github.com/mac-comp465-f18/basicgraphics
  cd basicgraphics
  mkdir build
  cd build
  cmake-gui ..
  ```
  Then configure, generate and build similar to MinVR.


### Installation
1. Clone the repositiory to your development directory:
```
git clone https://github.com/Fenmaz/you-animated.git
cd you-animated
```

2. Edit CMakeLists.txt, line 3, to point to your MinVR and basicgraphics installation
```
list(APPEND CMAKE_PREFIX_PATH ../basicgraphics/build/install ../minvr/build/install )
```

3. Build the project
```
mkdir build
cd build
cmake-gui ..
```
Then configure, generate and make the project.

## Development
The ```main``` branch currently renders the model without reading bone transformations and animations.
The ```bones``` branch is work in progress to render the model with animations.

## To-do
- Render the model with bones data and animation (branch ```bones```)
- Implement [optimized center of rotation skinning](https://dl.acm.org/citation.cfm?id=2925959)
