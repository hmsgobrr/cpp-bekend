# cpp-bekend
My another experiment on backend stuffs. This time I went a step higher and tried to make a web app with C++.
## Build Instructions
This project uses [cmake](https://cmake.org/). Altough projects that uses cmake are usually cross-platform, 
this project is can only be build and run on a Windows OS since this project uses a library that is used exclusively on each platform.
I'll add support for linux soon with linux's version of the library. So anyway, to build, first you need to clone this repo
like for example:
```
git clone https://github.com/hmsgobrr/cpp-bekend.git
```
Then make sure you have cmake version 3.0 or higher and run these commands (at the root folder).
You probably often see these commands on projects that uses cmake like this:
```
mkdir build                             # Create a new folder for cmake files
cd build                                # Change Directory into that folder
cmake -G Visual Studio 17 2022 ..       # Then generate the cmake files with your
                                        # preferred build system (options on: cmake --help)
                                        # which is Visual Studio in this command
```
After that you can build and run it with with the build system you chose.
