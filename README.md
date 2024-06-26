# Simulator
This repository contains the Monte Carlo Simulaton model and the Discrete Event Simulaton model.

The results from experiments conducted in the thesis can be found under the [data](https://github.com/Emergency-Optimizers/Simulator/tree/main/data) folder.

## Setup
### Required Directory Structure
To operate the program, ensure the following directory structure is in place:
```
- Emergency-Optimizers
    - Data-Processing
    - Simulator
```

`Emergency-Optimizers` serves as the root folder for both the [Data-Processing](https://github.com/Emergency-Optimizers/Data-Processing) repository and the Simulator repository.

You are required to have the enhanced OUH incident dataset as well as the depots CSV file under the Data-Processing repository.

### Windows
0. Install dependencies: CMake 3.27.7 and C++17 (MSVC)
1. Change directory `cd build`
2. Prepare cmake `cmake .. -DCMAKE_BUILD_TYPE=Release`
3. Build program `cmake --build .`
4. Run program `bin/Debug/Simulator.exe`
    To run all `cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . && bin/Debug/Simulator.exe`
