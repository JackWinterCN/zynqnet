set -ex
if [ ! -d build ]; then
    mkdir build
fi

cd build
rm ./* -rf
cmake ../
cmake --build .
# make install