/opt/pkg/petalinux/components/yocto/buildtools/sysroots/x86_64-petalinux-linux/usr/bin/bootgen -image bitstream.bif -arch zynqmp -o xpu_core.bin -w
# # 1. 将 bin 文件放入固件搜索路径 (通常是 /lib/firmware)
# cp xpu_core.bin /lib/firmware/

# # 2. 触发加载
# echo xpu_core.bin > /sys/class/fpga_manager/fpga0/firmware