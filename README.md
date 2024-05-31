# VkNovel

#### A simple straight forward visual novel engine

## How to build the project

#### Basic Requirement: Have the vulkan sdk installed on the system for premake to query the library path
#### Make sure that vulkan 1.2 support is available

```cpp
//in the project directory
premake5 [buildTarget]
```

#### For early development, I am containing the main.cpp in the src folder itself.<br /> In later versions, I will switch to building a .lib file to be used in the game project
