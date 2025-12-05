# FlexiCache - RISC-V 动态代码管理系统

一个基于 QEMU RISC-V 环境的灵活缓存管理系统原型，模拟在快速指令内存（I-Mem）和慢速 DRAM 之间动态搬运代码的过程。

## 🎯 核心设计思想

FlexiCache 解决的是**异构内存系统**中的核心问题：如何在有限的快速存储（I-Mem）和大容量慢速存储（DRAM）之间高效地管理代码？

### 关键特性

- ✅ **链接脚本模拟硬件**：用自定义链接脚本将 QEMU 的统一内存"虚拟地"切分成 I-Mem 和 DRAM
- ✅ **运行时动态加载**：实现代码块的动态搬运、驱逐和缓存管理
- ✅ **源级插桩**：使用 `CALL_MANAGED` 宏模拟预处理器的自动插桩功能
- ✅ **统计信息**：跟踪命中率、未命中率、搬运字节数等关键指标
- ✅ **完全容器化**：Docker 环境一键构建，无需本地安装工具链

## 📁 项目结构

```
flexicache/
├── Dockerfile              # 构建包含 RISC-V 工具链和 QEMU 的 Docker 镜像
├── Makefile                # 一键编译、运行、调试脚本
├── README.md               # 项目说明（本文件）
├── scripts/
│   └── flexicache.ld       # [核心] 自定义链接脚本 (定义 I-Mem vs DRAM)
├── runtime/
│   ├── flexicache.c        # [核心] 运行时库 (搬运工逻辑)
│   └── flexicache.h        # 头文件和 API 定义
└── src/
    └── main.c              # 用户测试程序（演示 FlexiCache 功能）
```

## 🚀 快速开始

### 方法 1: 使用 Docker（推荐）

这是最简单的方式，适用于 Windows、macOS 和 Linux。

```bash
# 1. 构建 Docker 镜像（首次使用或代码更新后）
docker build -t flexicache-env .

# 2. 进入容器环境
docker run -it --rm -v $(pwd):/workspace flexicache-env

# 3. 在容器内编译并运行
make          # 编译项目
make run      # 在 QEMU 中运行

# 4. 退出 QEMU：按 Ctrl+A, 然后按 X
```

### 方法 2: 本地编译（需要工具链）

如果您已经安装了 RISC-V 工具链：

```bash
# 1. 编译
make

# 2. 运行
make run

# 3. 查看反汇编（可选）
make disasm

# 4. 清理
make clean
```

## 🔧 核心组件详解

### 1. 链接脚本 (`scripts/flexicache.ld`)

**作用**：将 QEMU virt 板的 RAM（起始于 `0x80000000`）人为切分成两个区域：

- **I-Mem**: `0x80000000 - 0x800FFFFF` (1MB) - 存放运行时库和关键代码
- **DRAM**: `0x80100000 - 0x801FFFFF` (1MB) - 存放用户代码和数据

**关键代码**：

```ld
MEMORY
{
    IMEM (rwx) : ORIGIN = 0x80000000, LENGTH = 1M
    DRAM (rwx) : ORIGIN = 0x80100000, LENGTH = 1M
}
```

### 2. 运行时库 (`runtime/flexicache.c`)

**作用**：实现代码块的搬运逻辑，模拟缓存管理系统。

**核心 API**：

```c
void flexicache_init(void);                          // 初始化
int flexicache_load_block(void *func_addr, size_t);  // 加载代码块
int flexicache_evict_block(size_t size);             // 驱逐代码块
void flexicache_flush_icache(void);                  // 刷新指令缓存
```

**工作流程**：
1. 检查函数是否在 I-Mem（命中/未命中判断）
2. 如果未命中，从 DRAM 拷贝到 I-Mem
3. 如果 I-Mem 空间不足，触发驱逐策略
4. 使用 RISC-V `fence.i` 指令刷新指令缓存

### 3. 插桩宏 (`CALL_MANAGED`)

在完整系统中，应该由二进制重写器自动插入搬运逻辑。在此原型中，我们用宏模拟：

```c
// 使用方法
int result = CALL_MANAGED(fibonacci, 10);

// 等价于
flexicache_load_block((void*)fibonacci, 256);
int result = fibonacci(10);
```

### 4. 测试程序 (`src/main.c`)

演示程序包含多个测试用例：
- `fibonacci(10)` - 首次调用，触发加载
- `fibonacci(15)` - 再次调用，应该命中
- `factorial(8)` - 新函数，触发新的加载
- `sum_array()` - 数组操作测试

## 📊 预期输出

运行 `make run` 后，您应该看到类似的输出：

```
========================================
   FlexiCache 演示程序
========================================

[FlexiCache] 初始化完成
[FlexiCache] I-Mem: 0x80000000 - 0x80100000
[FlexiCache] DRAM:  0x80100000 - 0x80200000

[测试 1] 调用 fibonacci(10) - 首次调用（未命中）
[FlexiCache] 加载代码块: 0x80100XXX -> 0x80010000 (256 字节)
结果: 55

[测试 2] 再次调用 fibonacci(15) - 应该命中
结果: 610

========== FlexiCache 统计信息 ==========
加载次数: 0x00000004
驱逐次数: 0x00000000
命中次数: 0x00000001
未命中次数: 0x00000003
总搬运字节数: 0x00000400
=========================================

✓ 所有测试通过！
```

## 🐛 调试模式

FlexiCache 支持 GDB 调试：

```bash
# 终端 1: 启动 QEMU 调试模式
make debug

# 终端 2: 连接 GDB
gdb-multiarch flexicache_demo.elf
(gdb) target remote :1234
(gdb) b flexicache_load_block    # 设置断点
(gdb) c                          # 继续运行
(gdb) info registers             # 查看寄存器
(gdb) x/10i $pc                  # 查看当前指令
```

## 📝 技术细节

### 内存布局

| 区域 | 起始地址 | 大小 | 用途 |
|------|----------|------|------|
| I-Mem (运行时) | 0x80000000 | ~64KB | 运行时库代码 |
| I-Mem (缓存区) | 0x80010000 | ~960KB | 动态加载的代码 |
| DRAM (代码) | 0x80100000 | 可变 | 用户函数 |
| DRAM (数据) | 0x801XXXXX | 可变 | 全局变量 |
| DRAM (堆) | 0x801YXXXX | 64KB | 动态分配 |
| DRAM (栈) | 0x801FXXXX | 64KB | 函数调用栈 |

### 编译选项

- **架构**: RV32IMA (32位 RISC-V，整数+乘法+原子指令)
- **ABI**: ILP32 (整数和指针都是32位)
- **优化**: `-O2` (平衡速度和调试能力)
- **特殊选项**: `-nostdlib -ffreestanding` (裸机程序)

## 🎓 适用场景

这个原型适合作为：

1. **课程项目**：演示操作系统/计算机架构中的内存管理概念
2. **研究原型**：验证动态代码管理算法
3. **学习材料**：理解链接脚本、运行时系统、RISC-V 汇编
4. **概念验证**：在申请专利/论文前快速验证想法

## ⚠️ 当前限制

这是一个 **MVP (最小可行产品)**，存在以下简化：

1. **简单的驱逐策略**：目前直接清空整个 I-Mem，未实现 LRU/FIFO
2. **静态大小假设**：`CALL_MANAGED` 宏假设每个函数 256 字节
3. **无地址翻译**：直接拷贝代码，未处理位置无关代码(PIC)问题
4. **单线程**：未考虑并发访问

## 🚧 后续扩展方向

如果您有更多时间，可以考虑：

- [ ] 实现真正的 LRU 缓存替换算法
- [ ] 添加 Python 脚本自动分析 ELF 文件，确定函数大小
- [ ] 支持位置无关代码(PIC)，实现真正的地址重定位
- [ ] 添加性能分析工具，生成执行热图
- [ ] 实现预取(Prefetch)策略，提前加载可能用到的函数

## 📚 参考资料

- [RISC-V 指令集手册](https://riscv.org/technical/specifications/)
- [QEMU RISC-V 文档](https://www.qemu.org/docs/master/system/target-riscv.html)
- [GNU 链接脚本语法](https://sourceware.org/binutils/docs/ld/Scripts.html)

## 📄 许可证

MIT License - 可自由用于学术和商业项目

## 🙋 问题反馈

如果遇到问题，请检查：
1. Docker 是否正确安装？ (`docker --version`)
2. 是否在项目根目录运行命令？
3. 查看 `flexicache_demo.map` 文件检查内存布局

---

**祝您使用愉快！如果这个项目对您有帮助，请给个 ⭐️**

