set -ex

install_dst="$HOME/workspace/buildenv/temp/MNN/source/backend/xpu/backend/runtime"

if [ ! -d ${install_dst} ]; then
  mkdir -p ${install_dst}
fi
# rm ${install_dst}/* -rf
cp ./axilite.* ./shared_dram.* ./xfpga.* ./xfpga_hw.* ./xpu_core_wrapper.* \
   ../_HLS_CODE/fpga_top.* ../_HLS_CODE/gpool_cache.* ../_HLS_CODE/image_cache.* \
   ../_HLS_CODE/memory_controller.* ../_HLS_CODE/netconfig.* ../_HLS_CODE/network.* \
   ../_HLS_CODE/output_cache.* ../_HLS_CODE/processing_element.* \
   ../_HLS_CODE/weights_cache.* ${install_dst}
cp -r ../_HLS_CODE/vivado_include ${install_dst}
cd ${install_dst}

