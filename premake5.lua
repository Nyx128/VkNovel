local vulkanSdkPath = os.getenv("VULKAN_SDK")

if vulkanSdkPath == nil then
  error("The VULKAN_SDK environment variable is not set.")
end

local vulkanLibDir = vulkanSdkPath .. "\\Lib"
local vulkanIncludeDir = vulkanSdkPath .. "\\Include"

local projectDir = os.getcwd()

workspace "VkNovel"
  filename "VkNovel"
  configurations {"Debug", "Release"}
  platforms{"x64"}

project "VkNovel"
  kind "ConsoleApp"
  language "C++"

  cppdialect "C++17"

  files{
    "src/**.cpp",
    "src/**.hpp",
    "include/**.h",
	"include/**.hpp",
	"include/**.inl"
  }

  includedirs{vulkanIncludeDir, projectDir .. "\\include", projectDir .. "\\src"}
  libdirs{projectDir .. "\\lib", vulkanLibDir}

  links{"vulkan-1", "glfw3"}

  filter "configurations:Debug"
    defines { "_DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "platforms:x64"
    architecture "x64"
    

