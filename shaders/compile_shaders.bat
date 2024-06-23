@echo on

REM Check if VULKAN_SDK is set
if defined VULKAN_SDK (
    echo The VULKAN_SDK path is: %VULKAN_SDK%
    echo/
) else (
    echo VULKAN_SDK is not set.
    exit
)

set workDir=%cd%
echo %workDir%
set glslc="%VULKAN_SDK%\Bin\glslc.exe"
echo/

echo glslc is at %glslc%

for %%f in (%workDir%\*.vert) do (
    echo compiling file: %%f
    call %glslc% %%f -o %%f.spv
    echo/
)

for %%f in (%workDir%\*.frag) do (
    echo compiling file: %%f
    call %glslc% %%f -o %%f.spv
    echo/
)
