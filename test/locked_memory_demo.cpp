#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>

// 自定义异常类（C++风格错误处理）
class LockedMemoryException : public std::runtime_error {
public:
    explicit LockedMemoryException(const std::string& msg) : std::runtime_error(msg) {}
    LockedMemoryException(const std::string& msg, int err_code) 
        : std::runtime_error(msg + " (errno: " + std::to_string(err_code) + " - " + strerror(err_code) + ")") {}
};

// 内存权限结构体（C++封装）
struct MemoryPermissions {
    bool read = false;
    bool write = false;
    bool exec = false;
    bool shared = false;

    // 重载输出运算符，方便打印
    friend std::ostream& operator<<(std::ostream& os, const MemoryPermissions& perm) {
        os << "Permissions: "
           << (perm.read ? "R" : "-")
           << (perm.write ? "W" : "-")
           << (perm.exec ? "X" : "-")
           << (perm.shared ? "S" : "P");
        return os;
    }
};

/**
 * @brief 锁定内存管理器（纯原生Linux实现）
 * 功能：申请不分页/不交换的内存、获取物理地址、修改权限、释放内存
 */
class LockedMemoryManager {
public:
    // 禁止默认构造和拷贝
    LockedMemoryManager() = delete;
    LockedMemoryManager(const LockedMemoryManager&) = delete;
    LockedMemoryManager& operator=(const LockedMemoryManager&) = delete;

    /**
     * @brief 申请锁定内存（原生Linux方式：posix_memalign + mlock）
     * @param size 内存大小（字节，会自动页对齐）
     * @return 虚拟内存地址
     */
    static void* allocate(size_t size) {
        // 1. 获取系统页大小
        const size_t page_size = getPageSize();
        // 2. 页对齐处理（向上取整到页大小的整数倍）
        size = (size + page_size - 1) & ~(page_size - 1); 

        // 3. 页对齐分配内存
        void* ptr = nullptr;
        int ret = posix_memalign(&ptr, page_size, size);
        if (ret != 0) {
            throw LockedMemoryException("posix_memalign failed", ret);
        }

        // 4. 锁定内存，禁止交换到磁盘（核心）
        if (mlock(ptr, size) == -1) {
            int err = errno;
            ::free(ptr); // 分配失败需释放
            throw LockedMemoryException("mlock failed (check memlock limits)", err);
        }

        // 5. 默认设置为读写权限（不可执行，安全）
        modifyPermissions(ptr, size, PROT_READ | PROT_WRITE);

        return ptr;
    }

    /**
     * @brief 获取虚拟地址对应的物理地址（需ROOT权限）
     * @param virt_addr 虚拟地址
     * @return 物理地址（64位）
     */
    static uint64_t getPhysicalAddress(void* virt_addr) {
        if (virt_addr == nullptr) {
            throw LockedMemoryException("Invalid virtual address (nullptr)");
        }

        const size_t page_size = getPageSize();
        const off_t pagemap_entry_size = 8; // 每个页项8字节

        // 打开/proc/self/pagemap（仅ROOT可读）
        int pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
        if (pagemap_fd < 0) {
            throw LockedMemoryException("Failed to open /proc/self/pagemap (need root?)", errno);
        }

        // 计算虚拟页起始地址和pagemap偏移
        uint64_t virt_page_addr = reinterpret_cast<uint64_t>(virt_addr) & ~(page_size - 1);
        off_t offset = (virt_page_addr / page_size) * pagemap_entry_size;

        // 定位到目标页项
        if (lseek(pagemap_fd, offset, SEEK_SET) == -1) {
            int err = errno;
            close(pagemap_fd);
            throw LockedMemoryException("Failed to lseek pagemap", err);
        }

        // 读取页项数据
        uint64_t pfn_entry = 0;
        ssize_t read_bytes = read(pagemap_fd, &pfn_entry, pagemap_entry_size);
        close(pagemap_fd);

        if (read_bytes != pagemap_entry_size) {
            throw LockedMemoryException("Failed to read pagemap entry", errno);
        }

        // 检查页是否存在（第63位为1表示在物理内存中）
        if (!(pfn_entry & (1ULL << 63))) {
            throw LockedMemoryException("Virtual page is swapped out (not in physical memory)");
        }

        // 提取物理页帧号（0-54位）并计算物理地址
        uint64_t pfn = pfn_entry & ((1ULL << 55) - 1);
        uint64_t phys_addr = pfn * page_size + (reinterpret_cast<uint64_t>(virt_addr) % page_size);

        return phys_addr;
    }

    /**
     * @brief 获取指定虚拟地址的内存权限
     * @param virt_addr 虚拟地址
     * @return 内存权限结构体
     */
    static MemoryPermissions getPermissions(void* virt_addr) {
        if (virt_addr == nullptr) {
            throw LockedMemoryException("Invalid virtual address (nullptr)");
        }

        std::ifstream maps_file("/proc/self/maps");
        if (!maps_file.is_open()) {
            throw LockedMemoryException("Failed to open /proc/self/maps", errno);
        }

        std::string line;
        uint64_t virt_addr_ull = reinterpret_cast<uint64_t>(virt_addr);
        MemoryPermissions perm;

        while (std::getline(maps_file, line)) {
            uint64_t start, end;
            char perms[5];
            // 解析格式：起始地址-结束地址 权限 偏移 设备 inode
            if (sscanf(line.c_str(), "%lx-%lx %4s", &start, &end, perms) >= 3) {
                if (virt_addr_ull >= start && virt_addr_ull < end) {
                    perm.read = (perms[0] == 'r');
                    perm.write = (perms[1] == 'w');
                    perm.exec = (perms[2] == 'x');
                    perm.shared = (perms[3] == 's');
                    break;
                }
            }
        }

        maps_file.close();
        return perm;
    }

    /**
     * @brief 修改内存权限
     * @param virt_addr 虚拟地址（页对齐）
     * @param size 内存大小（页对齐）
     * @param prot 新权限（PROT_READ/PROT_WRITE/PROT_EXEC组合）
     */
    static void modifyPermissions(void* virt_addr, size_t size, int prot) {
        if (virt_addr == nullptr) {
            throw LockedMemoryException("Invalid virtual address (nullptr)");
        }

        const size_t page_size = getPageSize();
        // 验证地址和大小是否页对齐（否则mprotect会失败）
        if (reinterpret_cast<uint64_t>(virt_addr) % page_size != 0 || size % page_size != 0) {
            throw LockedMemoryException("Address or size is not page-aligned");
        }

        if (mprotect(virt_addr, size, prot) == -1) {
            throw LockedMemoryException("Failed to modify memory permissions", errno);
        }
    }

    /**
     * @brief 释放锁定的内存
     * @param virt_addr 虚拟地址
     * @param size 内存大小（页对齐）
     */
    static void free(void* virt_addr, size_t size) {
        if (virt_addr == nullptr) return;

        // 1. 解锁内存（解除mlock限制）
        munlock(virt_addr, size);
        // 2. 释放内存
        ::free(virt_addr);
    }

    /**
     * @brief 获取系统页大小
     * @return 页大小（字节）
     */
    static size_t getPageSize() {
        static size_t page_size = sysconf(_SC_PAGESIZE);
        if (page_size == -1) {
            throw LockedMemoryException("Failed to get page size", errno);
        }
        return page_size;
    }
};

// 辅助函数：打印内存信息
void printMemoryInfo(void* virt_addr, size_t size) {
    std::cout << "\n=== Memory Information ===" << std::endl;
    std::cout << "Virtual Address: " << virt_addr << std::endl;
    std::cout << "Size: " << size / (1024 * 1024) << " MB (page-aligned)" << std::endl;
    
    // 获取物理地址（需ROOT）
    try {
        uint64_t phys_addr = LockedMemoryManager::getPhysicalAddress(virt_addr);
        std::cout << "Physical Address: 0x" << std::hex << phys_addr << std::dec << std::endl;
    } catch (const LockedMemoryException& e) {
        std::cerr << "Warning: Failed to get physical address: " << e.what() << std::endl;
    }

    // 获取权限
    MemoryPermissions perm = LockedMemoryManager::getPermissions(virt_addr);
    std::cout << perm << std::endl;
}

int main() {
    try {
        // 1. 配置参数（可根据需求调整内存大小）
        const size_t MEM_SIZE = 64 * 1024 * 1024; // 64MB

        // 2. 打印系统信息
        std::cout << "=== System Info ===" << std::endl;
        std::cout << "Page Size: " << LockedMemoryManager::getPageSize() / 1024 << " KB" << std::endl;
        std::cout << "[warning] Running as Root: " << (getuid() == 0 ? "Yes" : "No (physical address may fail)") << std::endl;

        // 3. 申请锁定内存（不分页、不交换）
        std::cout << "\n=== Allocating Locked Memory (Native Linux Mode) ===" << std::endl;
        void* virt_mem = LockedMemoryManager::allocate(MEM_SIZE);
        std::cout << "Allocated locked memory at: " << virt_mem << std::endl;

        // 4. 打印内存详细信息
        printMemoryInfo(virt_mem, MEM_SIZE);

        // 5. 修改内存权限（演示添加执行权限）
        std::cout << "\n=== Modifying Memory Permissions ===" << std::endl;
        try {
            LockedMemoryManager::modifyPermissions(virt_mem, MEM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
            MemoryPermissions new_perm = LockedMemoryManager::getPermissions(virt_mem);
            std::cout << "New Permissions: " << new_perm << std::endl;
        } catch (const LockedMemoryException& e) {
            std::cerr << "Warning: Failed to add execute permission: " << e.what() << std::endl;
        }

        // 验证内存可写
        uint32_t test_value = 0x12345678;
        *reinterpret_cast<uint32_t*>(virt_mem) = test_value;
        uint32_t read_back = *reinterpret_cast<uint32_t*>(virt_mem);
        std::cout << "Write/Read Test: " << (read_back == test_value ? "Success" : "Failed") 
                << " (Wrote: 0x" << std::hex << test_value << ", Read: 0x" << read_back << std::dec << ")" << std::endl;

        // 6. 释放内存
        std::cout << "\n=== Freeing Locked Memory ===" << std::endl;
        LockedMemoryManager::free(virt_mem, MEM_SIZE);
        std::cout << "Memory freed successfully" << std::endl;

    } catch (const LockedMemoryException& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\nDemo completed successfully!" << std::endl;
    return EXIT_SUCCESS;
}