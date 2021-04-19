cd build &&
    cmake .. &&
    make -j5 &&
    cd ..

./build/mli $1

