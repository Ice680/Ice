# Ice

## Testing
> I know there's a much better and simpler way to do this. But this works for now. - Ice680

```sh 
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B build -G Ninja -DCMAKE_LINKER=ld.lld -DCMAKE_CXX_LINK_EXECUTABLE="<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
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
