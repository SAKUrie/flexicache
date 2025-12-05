# 📤 将 FlexiCache 发布到 GitHub

本指南将帮助您将 FlexiCache 项目发布到 GitHub。

## 🎯 方法一：使用 GitHub Web 界面（推荐）

### 步骤 1: 在 GitHub 创建新仓库

1. 访问 [https://github.com/new](https://github.com/new)
2. 填写仓库信息：
   - **Repository name**: `flexicache` 或 `FlexiCache-RISCV`
   - **Description**: `A dynamic code management system for RISC-V with heterogeneous memory (I-Mem + DRAM) simulation`
   - **Visibility**: 选择 Public（公开）或 Private（私有）
   - ⚠️ **不要勾选** "Initialize this repository with a README"（我们已经有了）
3. 点击 **"Create repository"**

### 步骤 2: 推送代码到 GitHub

在您的终端中，依次执行以下命令：

```bash
# 确保在项目目录中
cd /Users/sakurie/PycharmProjects/554/flexicache

# 添加远程仓库（替换 YOUR_USERNAME 为您的 GitHub 用户名）
git remote add origin https://github.com/YOUR_USERNAME/flexicache.git

# 推送代码
git branch -M main
git push -u origin main
```

**示例**（假设您的用户名是 `sakurie`）:
```bash
git remote add origin https://github.com/sakurie/flexicache.git
git branch -M main
git push -u origin main
```

### 步骤 3: 验证发布成功

访问您的仓库页面，应该能看到：
- ✅ 所有文件已上传
- ✅ README.md 自动显示在首页
- ✅ 项目结构清晰可见

---

## 🎯 方法二：使用 GitHub CLI（高级用户）

如果您安装了 GitHub CLI (`gh`)：

```bash
# 在项目目录中
cd /Users/sakurie/PycharmProjects/554/flexicache

# 创建仓库并推送（一步完成）
gh repo create flexicache --public --source=. --push --description "A dynamic code management system for RISC-V"
```

---

## 📝 建议的仓库设置

### 添加 Topics（标签）

在 GitHub 仓库页面，点击右侧的 ⚙️ 设置图标，添加以下 topics：

- `riscv`
- `qemu`
- `docker`
- `cache-management`
- `embedded-systems`
- `memory-management`
- `computer-architecture`
- `linker-script`

### 设置 About

在仓库描述中添加：

```
🚀 A dynamic code management system simulating heterogeneous memory (I-Mem + DRAM) on RISC-V architecture using Docker + QEMU
```

### 添加 License

如果您想添加开源协议：

```bash
# MIT License（最常用的宽松许可证）
curl -o LICENSE https://raw.githubusercontent.com/licenses/license-templates/master/templates/mit.txt

# 编辑 LICENSE 文件，填写您的名字和年份
# 然后提交
git add LICENSE
git commit -m "📄 Add MIT License"
git push
```

---

## 🎨 美化 README（可选）

您可以在 README.md 顶部添加徽章（badges）：

```markdown
# FlexiCache - RISC-V 动态代码管理系统

[![Docker](https://img.shields.io/badge/docker-ready-blue.svg)](https://www.docker.com/)
[![RISC-V](https://img.shields.io/badge/RISC--V-RV32IMA-green.svg)](https://riscv.org/)
[![QEMU](https://img.shields.io/badge/QEMU-virt-orange.svg)](https://www.qemu.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
```

---

## 🔄 后续更新

当您修改代码后，使用以下命令提交并推送：

```bash
# 查看修改
git status

# 添加所有修改
git add .

# 提交修改
git commit -m "描述您的修改"

# 推送到 GitHub
git push
```

**提交信息建议**：
- `✨ feat: 添加新功能`
- `🐛 fix: 修复 bug`
- `📝 docs: 更新文档`
- `⚡️ perf: 性能优化`
- `🎨 style: 代码格式化`
- `♻️ refactor: 重构代码`

---

## 🌟 推广您的项目

### 1. 添加项目截图

在 README.md 中添加运行截图：

```bash
# 在项目根目录创建 assets 文件夹
mkdir assets

# 将截图放入 assets/ 文件夹
# 然后在 README.md 中引用：
# ![运行效果](assets/screenshot.png)
```

### 2. 创建 GitHub Release

当项目稳定后，创建一个发布版本：

```bash
# 创建标签
git tag -a v1.0.0 -m "First stable release"
git push origin v1.0.0

# 然后在 GitHub 网页上创建 Release
```

### 3. 分享到社区

- Reddit: r/RISCV, r/embedded
- Twitter/X: 使用 #RISCV #QEMU 标签
- Hacker News: 如果项目有独特之处

---

## ⚠️ 常见问题

### Q1: 推送时要求输入用户名和密码

**解决方案**: GitHub 已不再支持密码验证，需要使用 Personal Access Token (PAT)。

1. 访问 [https://github.com/settings/tokens](https://github.com/settings/tokens)
2. 点击 "Generate new token (classic)"
3. 勾选 `repo` 权限
4. 复制生成的 token
5. 在命令行输入密码时，粘贴 token（不是您的 GitHub 密码）

### Q2: 推送被拒绝（rejected）

**原因**: 远程仓库可能已有内容

**解决方案**:
```bash
git pull origin main --rebase
git push origin main
```

### Q3: 想要修改仓库名称

在 GitHub 网页上：
1. 进入仓库页面
2. 点击 Settings
3. 修改 Repository name
4. 在本地更新远程地址：
```bash
git remote set-url origin https://github.com/YOUR_USERNAME/NEW_NAME.git
```

---

## 📊 当前 Git 状态

```bash
# 查看当前状态
git log --oneline -5

# 查看远程仓库
git remote -v

# 查看所有文件
git ls-files
```

---

## ✅ 发布检查清单

在推送到 GitHub 前，确认：

- [ ] 所有文件已添加到 Git（`git status` 无未跟踪文件）
- [ ] README.md 内容完整且格式正确
- [ ] .gitignore 已正确配置（不包含编译产物）
- [ ] 项目可以成功编译运行（`make clean && make run`）
- [ ] 文档拼写无误
- [ ] 删除了敏感信息（如果有）

---

## 🎉 完成！

一旦推送成功，您的项目将对全世界可见！

**项目 URL**: `https://github.com/YOUR_USERNAME/flexicache`

记得在论文、报告或简历中引用这个 GitHub 链接！📚

---

## 📞 需要帮助？

- GitHub 文档: [https://docs.github.com](https://docs.github.com)
- Git 教程: [https://git-scm.com/book/zh/v2](https://git-scm.com/book/zh/v2)

