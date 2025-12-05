# FlexiCache 快速入门指南 🚀

本指南将带您在 **3 分钟内**完成 FlexiCache 的构建和运行。

## 📋 前置要求

- Docker（[安装指南](https://docs.docker.com/get-docker/)）
- 终端/命令行工具
- 至少 2GB 磁盘空间

## ⚡️ 快速开始（三步走）

### 步骤 1: 构建 Docker 环境

在 `flexicache/` 目录下执行：

```bash
docker build -t flexicache-env .
```

**预计时间**: 2-5 分钟（首次构建）

**预期输出**:
```
Successfully built xxxxx
Successfully tagged flexicache-env:latest
```

### 步骤 2: 进入开发容器

```bash
# macOS/Linux
docker run -it --rm -v $(pwd):/workspace flexicache-env

# Windows (PowerShell)
docker run -it --rm -v ${PWD}:/workspace flexicache-env

# Windows (CMD)
docker run -it --rm -v %cd%:/workspace flexicache-env
```

现在您已经进入了配置好的 RISC-V 开发环境！

### 步骤 3: 编译并运行

在容器内执行：

```bash
make          # 编译项目
make run      # 在 QEMU 中运行
```

**退出 QEMU**: 按 `Ctrl+A`，然后按 `X`

## 🎯 预期输出示例

运行成功后，您应该看到：

```
========================================
   FlexiCache 演示程序
========================================

[FlexiCache] 初始化完成
[FlexiCache] I-Mem: 0x80000000 - 0x80100000
[FlexiCache] DRAM:  0x80100000 - 0x80200000

[测试 1] 调用 fibonacci(10) - 首次调用（未命中）
[FlexiCache] 加载代码块: 0x80100XXX -> 0x80010000 (0x00000100 字节)
结果: 55

[测试 2] 再次调用 fibonacci(15) - 应该命中
结果: 610

[测试 3] 调用 factorial(8)
[FlexiCache] 加载代码块: 0x80100YYY -> 0x80010100 (0x00000100 字节)
结果: 40320

[测试 4] 调用 sum_array
[FlexiCache] 加载代码块: 0x80100ZZZ -> 0x80010200 (0x00000100 字节)
结果: 55

========== FlexiCache 统计信息 ==========
加载次数: 0x00000003
驱逐次数: 0x00000000
命中次数: 0x00000001
未命中次数: 0x00000003
总搬运字节数: 0x00000300
=========================================

========== 验证 ==========
✓ 所有测试通过！

程序执行完成。按 Ctrl+A, X 退出 QEMU。
```

## 🔍 深入探索

### 查看反汇编

```bash
make disasm
cat flexicache_demo.asm
```

### 查看内存映射

```bash
cat flexicache_demo.map
```

### 调试模式

在容器内打开两个终端：

**终端 1** - 启动 QEMU 调试模式:
```bash
make debug
```

**终端 2** - 连接 GDB:
```bash
gdb-multiarch flexicache_demo.elf
(gdb) target remote :1234
(gdb) b flexicache_load_block    # 在加载函数设置断点
(gdb) c                          # 继续执行
(gdb) info registers             # 查看寄存器
(gdb) x/10i $pc                  # 反汇编当前位置
(gdb) bt                         # 查看调用栈
```

常用 GDB 命令：
- `b <函数名>` - 设置断点
- `c` - 继续执行
- `n` - 单步执行（不进入函数）
- `s` - 单步执行（进入函数）
- `p <变量>` - 打印变量值
- `x/<数量><格式> <地址>` - 查看内存

## 🛠️ 修改代码

### 添加新的测试函数

1. 在 `src/main.c` 中添加函数：

```c
int my_test_function(int x) {
    return x * x + 10;
}
```

2. 在 `_start()` 函数中调用：

```c
int result = CALL_MANAGED(my_test_function, 42);
puts("结果: ");
putdec(result);
```

3. 重新编译并运行：

```bash
make clean
make run
```

## 📊 性能分析

### 查看统计信息

运行时库会自动跟踪：
- **加载次数**: 从 DRAM 搬运到 I-Mem 的次数
- **驱逐次数**: I-Mem 空间不足时的清理次数
- **命中次数**: 函数已在 I-Mem 中
- **未命中次数**: 需要从 DRAM 加载
- **总搬运字节数**: 累计数据传输量

### 计算命中率

```
命中率 = 命中次数 / (命中次数 + 未命中次数)
```

## ❓ 常见问题

### Q1: Docker 构建失败

**症状**: `apt-get install` 报错

**解决方案**:
```bash
# 使用国内镜像加速（中国大陆用户）
docker build --build-arg DEBIAN_FRONTEND=noninteractive -t flexicache-env .
```

### Q2: QEMU 无输出

**症状**: `make run` 后没有任何输出

**解决方案**:
1. 检查编译是否成功: `ls -lh flexicache_demo.elf`
2. 查看反汇编: `make disasm`
3. 使用调试模式: `make debug`

### Q3: 文件修改不生效

**症状**: 修改代码后运行结果没变化

**解决方案**:
```bash
make clean   # 清理旧文件
make         # 重新编译
```

### Q4: 如何退出 QEMU？

**方案 1** (推荐): `Ctrl+A`, 然后按 `X`
**方案 2**: 在另一个终端执行 `docker stop <容器ID>`
**方案 3**: 关闭终端窗口

## 📚 下一步

- 阅读 [README.md](./README.md) 了解技术细节
- 查看 [runtime/flexicache.c](./runtime/flexicache.c) 理解运行时逻辑
- 研究 [scripts/flexicache.ld](./scripts/flexicache.ld) 学习链接脚本
- 实验不同的驱逐策略（LRU, FIFO, Random）

## 🎓 学习资源

- [RISC-V 入门教程](https://riscv.org/learn/)
- [QEMU 文档](https://www.qemu.org/docs/master/)
- [GNU Linker Scripts](https://sourceware.org/binutils/docs/ld/Scripts.html)

---

**遇到问题？** 检查文件 `flexicache_demo.map` 查看内存布局是否正确。
