set -ex
cd build
rm ./* -rf
cmake ../
cmake --build .
make install