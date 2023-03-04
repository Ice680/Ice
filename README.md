# Ice
A x86_64 operating system

## Building and Testing
```sh 
$ cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B build -G Ninja"
```
```sh
$ ninja -C build run_qemu
```

## Dependencies
* CMake (version 3.12 or newer)
* LLVM + Clang + LLD (14.x+)
* Ninja-build
* QEMU system emulator
* GNU xorriso
