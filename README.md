# Simulator
This is the simulator used for evaluating emergency response times

## Setup
### Linux
0. Install dependencies `sudo apt-get install gcc g++ cmake`
1. Change directory `cd build`
2. Prepare cmake `cmake .. -DCMAKE_BUILD_TYPE=Release`
3. Build program `cmake --build .`
4. Run program `bin/Simulator`

### Windows
0. Install dependencies `winget install kitware.cmake`
1. Change directory `cd build`
2. Prepare cmake `cmake .. -DCMAKE_BUILD_TYPE=Release`
3. Build program `cmake --build .`
4. Run program `bin/Debug/Simulator.exe`
5. To run all `cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && bin/Debug/Simulator.exe`
