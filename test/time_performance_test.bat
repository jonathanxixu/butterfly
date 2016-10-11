@echo off
set root_dir=%~dp0\..
set image_dir=%~dp0
call "%root_dir%/bin/image_filter_test.exe" %image_dir%\test_image.bmp [1,2,4,7,11,17,25,32] 10 1
