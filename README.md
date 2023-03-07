# cpp-bekend
My another experiment on backend stuffs. This time I went a step higher and tried to make a web app with C++.
## Build Instructions
To build, first you need to clone this repo
like for example:
```
git clone https://github.com/hmsgobrr/cpp-bekend.git
```
Then, make sure you have [cmake](https://cmake.org/) version 3.0 or higher
and run these commands to generate the cmake files (in the project's folder).
```
mkdir build                 # Creates a new folder, "build" for the cmake files
cd build                    # Changes the Directory into that folder
cmake -G Ninja ..           # Finally, this generates the cmake files with your
                            # preferred build system (options on: cmake --help)
                            # which is my preffered, Ninja Build System in this command
```
After that you can build and run it with with the build system you chose.
For example if you chose Ninja you can run these commands (in the "build" folder) to run the program:
```
ninja                       # Builds the program. The output file is at build/cpp-bekend(.exe)
cpp-bekend.exe              # Runs the program. You can omit the .exe, you have to, if you are not using windows.
```
