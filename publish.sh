#!/bin/bash
# FlexiCache GitHub 快速发布脚本

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                                                              ║"
echo "║          FlexiCache GitHub 发布助手                          ║"
echo "║                                                              ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查是否已有远程仓库
if git remote | grep -q origin; then
    echo -e "${YELLOW}⚠️  检测到已存在 origin 远程仓库${NC}"
    echo ""
    git remote -v
    echo ""
    read -p "是否要移除现有的 origin 并重新配置？(y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git remote remove origin
        echo -e "${GREEN}✓ 已移除旧的 origin${NC}"
    else
        echo -e "${YELLOW}跳过配置，使用现有的 origin${NC}"
        echo ""
        echo "如果要推送，请运行:"
        echo -e "${BLUE}  git push -u origin main${NC}"
        exit 0
    fi
fi

# 获取用户输入
echo ""
echo "请输入您的 GitHub 信息:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
read -p "GitHub 用户名: " github_user

if [ -z "$github_user" ]; then
    echo -e "${RED}✗ 用户名不能为空${NC}"
    exit 1
fi

read -p "仓库名 [flexicache]: " repo_name
repo_name=${repo_name:-flexicache}

# 构建仓库 URL
repo_url="https://github.com/${github_user}/${repo_name}.git"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "将配置远程仓库:"
echo -e "${BLUE}${repo_url}${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
read -p "确认推送？(y/n) " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}已取消${NC}"
    exit 0
fi

# 添加远程仓库
echo ""
echo "🔗 配置远程仓库..."
if git remote add origin "$repo_url" 2>/dev/null; then
    echo -e "${GREEN}✓ 远程仓库配置成功${NC}"
else
    echo -e "${RED}✗ 配置失败（可能已存在）${NC}"
    exit 1
fi

# 确保在 main 分支
echo ""
echo "🔄 切换到 main 分支..."
git branch -M main
echo -e "${GREEN}✓ 已切换到 main 分支${NC}"

# 推送代码
echo ""
echo "📤 推送代码到 GitHub..."
echo ""
echo -e "${YELLOW}⚠️  如果要求输入密码，请使用 Personal Access Token (PAT)${NC}"
echo -e "${YELLOW}   获取 PAT: https://github.com/settings/tokens${NC}"
echo ""

if git push -u origin main; then
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo -e "${GREEN}🎉 发布成功！${NC}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "🌐 访问您的仓库:"
    echo -e "${BLUE}   https://github.com/${github_user}/${repo_name}${NC}"
    echo ""
    echo "📝 后续更新请使用:"
    echo "   git add ."
    echo "   git commit -m \"您的提交信息\""
    echo "   git push"
    echo ""
else
    echo ""
    echo -e "${RED}✗ 推送失败${NC}"
    echo ""
    echo "可能的原因:"
    echo "1. 仓库不存在 - 请先在 GitHub 创建仓库"
    echo "   访问: https://github.com/new"
    echo ""
    echo "2. 认证失败 - 请使用 Personal Access Token"
    echo "   获取: https://github.com/settings/tokens"
    echo ""
    echo "3. 远程已有内容 - 尝试先拉取:"
    echo "   git pull origin main --rebase"
    echo "   git push origin main"
    echo ""
    exit 1
fi

