set build_dir=%~dp0\vs2013x64
set src_dir=%~dp0\..

if NOT EXIST %build_dir% (mkdir %build_dir%)

pushd %build_dir%
cmake -G "Visual Studio 12 Win64" %src_dir%
popd
