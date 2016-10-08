@echo off
set root_dir=%~dp0\..
set image_dir=%~dp0
::Usage: ImageFilter.exe {image} {radius array, e.g.[1,3,5,9,15,21,31]} {run_times} {save image:1 , not:0(default)}
call "%root_dir%/build/vs2013x64/bin/Release/ImageFilter.exe" %image_dir%\test_image.bmp [1,2,4,7,11,17,25,32] 10 1
