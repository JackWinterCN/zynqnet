export PATH=/tools/Xilinx/Vitis/2020.1/gnu/aarch64/lin/aarch64-linux/bin/:$PATH
cd build_zynq

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=./toolchain_zynq_aarch64.cmake \
  -DCMAKE_BUILD_TYPE=Debug            \
  -DMNN_BUILD_DEMO=ON                 \
  -DMNN_BUILD_OPENCV=ON               \
  -DMNN_LOW_MEMORY=ON

#  -DMNN_XPU=true                      \

make -j4
