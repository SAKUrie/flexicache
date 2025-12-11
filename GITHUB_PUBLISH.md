# 📤 Publishing FlexiCache to GitHub

This guide will help you publish the FlexiCache project to GitHub.

## 🎯 Method 1: Using GitHub Web Interface (Recommended)

### Step 1: Create a New Repository on GitHub

1. Visit [https://github.com/new](https://github.com/new)
2. Fill in repository information:
   - **Repository name**: `flexicache` or `FlexiCache-RISCV`
   - **Description**: `A dynamic code management system for RISC-V with heterogeneous memory (I-Mem + DRAM) simulation`
   - **Visibility**: Choose Public or Private
   - ⚠️ **Do NOT check** "Initialize this repository with a README" (we already have one)
3. Click **"Create repository"**

### Step 2: Push Code to GitHub

In your terminal, execute the following commands:

```bash
# Make sure you're in the project directory
cd /Users/sakurie/PycharmProjects/554/flexicache

# Add remote repository (replace YOUR_USERNAME with your GitHub username)
git remote add origin https://github.com/YOUR_USERNAME/flexicache.git

# Push code
git branch -M main
git push -u origin main
```

**Example** (assuming your username is `sakurie`):
```bash
git remote add origin https://github.com/sakurie/flexicache.git
git branch -M main
git push -u origin main
```

### Step 3: Verify Publication Success

Visit your repository page, you should see:
- ✅ All files uploaded
- ✅ README.md automatically displayed on homepage
- ✅ Project structure clearly visible

---

## 🎯 Method 2: Using GitHub CLI (Advanced Users)

If you have GitHub CLI (`gh`) installed:

```bash
# In project directory
cd /Users/sakurie/PycharmProjects/554/flexicache

# Create repository and push (one step)
gh repo create flexicache --public --source=. --push --description "A dynamic code management system for RISC-V"
```

---

## 📝 Recommended Repository Settings

### Add Topics (Tags)

On the GitHub repository page, click the ⚙️ settings icon on the right, add the following topics:

- `riscv`
- `qemu`
- `docker`
- `cache-management`
- `embedded-systems`
- `memory-management`
- `computer-architecture`
- `linker-script`

### Set About

Add to repository description:

```
🚀 A dynamic code management system simulating heterogeneous memory (I-Mem + DRAM) on RISC-V architecture using Docker + QEMU
```

### Add License

If you want to add an open source license:

```bash
# MIT License (most common permissive license)
curl -o LICENSE https://raw.githubusercontent.com/licenses/license-templates/master/templates/mit.txt

# Edit LICENSE file, fill in your name and year
# Then commit
git add LICENSE
git commit -m "📄 Add MIT License"
git push
```

---

## 🎨 Beautify README (Optional)

You can add badges at the top of README.md:

```markdown
# FlexiCache - RISC-V Dynamic Code Management System

[![Docker](https://img.shields.io/badge/docker-ready-blue.svg)](https://www.docker.com/)
[![RISC-V](https://img.shields.io/badge/RISC--V-RV32IMA-green.svg)](https://riscv.org/)
[![QEMU](https://img.shields.io/badge/QEMU-virt-orange.svg)](https://www.qemu.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
```

---

## 🔄 Subsequent Updates

After modifying code, use the following commands to commit and push:

```bash
# View changes
git status

# Add all changes
git add .

# Commit changes
git commit -m "Describe your changes"

# Push to GitHub
git push
```

**Commit message suggestions**:
- `✨ feat: Add new feature`
- `🐛 fix: Fix bug`
- `📝 docs: Update documentation`
- `⚡️ perf: Performance optimization`
- `🎨 style: Code formatting`
- `♻️ refactor: Refactor code`

---

## 🌟 Promote Your Project

### 1. Add Project Screenshots

Add screenshots to README.md:

```bash
# Create assets folder in project root
mkdir assets

# Put screenshots in assets/ folder
# Then reference in README.md:
# ![Demo](assets/screenshot.png)
```

### 2. Create GitHub Release

When project is stable, create a release version:

```bash
# Create tag
git tag -a v1.0.0 -m "First stable release"
git push origin v1.0.0

# Then create Release on GitHub website
```

### 3. Share to Community

- Reddit: r/RISCV, r/embedded
- Twitter/X: Use #RISCV #QEMU tags
- Hacker News: If project has unique features

---

## ⚠️ Frequently Asked Questions

### Q1: Asked for username and password when pushing

**Solution**: GitHub no longer supports password authentication, need to use Personal Access Token (PAT).

1. Visit [https://github.com/settings/tokens](https://github.com/settings/tokens)
2. Click "Generate new token (classic)"
3. Check `repo` permission
4. Copy the generated token
5. When prompted for password in command line, paste the token (not your GitHub password)

### Q2: Push rejected

**Reason**: Remote repository may already have content

**Solution**:
```bash
git pull origin main --rebase
git push origin main
```

### Q3: Want to change repository name

On GitHub website:
1. Go to repository page
2. Click Settings
3. Modify Repository name
4. Update remote URL locally:
```bash
git remote set-url origin https://github.com/YOUR_USERNAME/NEW_NAME.git
```

---

## 📊 Current Git Status

```bash
# View current status
git log --oneline -5

# View remote repository
git remote -v

# View all files
git ls-files
```

---

## ✅ Publication Checklist

Before pushing to GitHub, confirm:

- [ ] All files added to Git (`git status` shows no untracked files)
- [ ] README.md content complete and formatted correctly
- [ ] .gitignore properly configured (excludes build artifacts)
- [ ] Project compiles and runs successfully (`make clean && make run`)
- [ ] Documentation spelling verified
- [ ] Sensitive information removed (if any)

---

## 🎉 Done!

Once pushed successfully, your project will be visible worldwide!

**Project URL**: `https://github.com/YOUR_USERNAME/flexicache`

Remember to cite this GitHub link in papers, reports, or resumes! 📚

---

## 📞 Need Help?

- GitHub Documentation: [https://docs.github.com](https://docs.github.com)
- Git Tutorial: [https://git-scm.com/book/en/v2](https://git-scm.com/book/en/v2)
