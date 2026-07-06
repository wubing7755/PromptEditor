# PromptEditor - 纯 C Prompt 管理 CLI

> [English](README.md)

在本地文件夹中保存、组织、搜索、浏览和优化 prompt 模板 - 纯 C 实现，零外部依赖。

## 特性

- **存储 prompt** 在本地文件夹中 - 纯文本文件，无需数据库
- **组织管理** 按文件夹、分类和标签
- **交互式搜索** 使用 `pp browse`（如有 `fzf` 则使用模糊搜索，否则使用编号菜单）
- **编辑器编辑** 使用 `pp edit <id>` 在 `$EDITOR` 中编辑 prompt 内容
- **版本管理** 使用 `pp optimize` 创建优化版本，支持历史对比和版本晋升
- **导出/导入/备份** 完整库操作
- **JSON 输出** 使用 `--json` 标志，适合脚本处理
- **彩色终端输出** 自动分页（遵循 `NO_COLOR` 规范）
- **跨平台** - Windows (MinGW/MSVC)、Linux、macOS

## 快速开始

```sh
# 克隆并构建（Windows PowerShell 请使用 .ps1 脚本）
git clone https://github.com/your-username/PromptEditor
cd PromptEditor
./scripts/bootstrap.sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

初始化库并保存第一个 prompt：

```sh
./build/ninja-debug/bin/pp init
./build/ninja-debug/bin/pp add --title "Hello" --body "这是我的第一个 prompt。"
./build/ninja-debug/bin/pp list
```

> **提示：** 默认库根目录为 `~/.promptlib` (Linux/macOS) 或
> `%USERPROFILE%\.promptlib` (Windows)。可通过 `--root <path>` 或
> `PROMPTLIB_ROOT` 环境变量覆盖。

## 命令列表

| 命令 | 说明 |
|---------|------|
| `help` | 显示全局帮助 |
| `init` | 初始化 prompt 库文件夹 |
| `add` | 保存新 prompt（支持 `--editor` 打开 `$EDITOR`） |
| `list` | 列出 prompt（`--json`、`--no-pager`，支持按文件夹/分类/标签过滤） |
| `show` | 按 ID 或标题显示 prompt（`--raw`、`--json`） |
| `edit` | 编辑 prompt 元数据或内容（不传标志时打开 `$EDITOR`） |
| `delete` | 归档或永久删除 prompt |
| `search` | 搜索标题、内容、标签和描述（`--json`、`--raw`） |
| `browse` | 交互式浏览器 - 有 `fzf` 时使用模糊搜索+预览，否则使用编号菜单 |
| `optimize` | 创建带版本管理的优化版本，支持历史查看和对比 |
| `folder` | 管理文件夹（`list`、`create`、`remove`、`rename`） |
| `category` | 管理分类（`list`、`create`、`remove`、`rename`） |
| `export` | 导出 prompt 或子目录到另一个文件夹 |
| `import` | 从导出目录或其他库导入 prompt |
| `backup` | 创建完整的库备份 |

任何命令加 `--help` 可查看详细用法。

## 使用示例

```sh
# 初始化库并添加 prompt
pp init --root ./my-prompts
pp add --title "Summarize" --editor --root ./my-prompts
pp add --title "Translate" --body "请将这段文字翻译成中文。" --root ./my-prompts

# 列出和查看
pp list --root ./my-prompts
pp show --root ./my-prompts "translate"

# 搜索、编辑和管理
pp search "Chinese" --root ./my-prompts
pp edit --root ./my-prompts "translate" --title "Renamed"
pp delete --root ./my-prompts "translate" --yes
```

## 构建与测试

```sh
# 运行完整检查 (配置、构建、测试)
./scripts/check.sh      # Windows 上使用 .\scripts\check.ps1

# 或逐步执行
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

支持的编译器：GCC 8+、Clang 7+、MSVC 2019+。详见 [doc/guides/cmake.md](doc/guides/cmake.md)。

## 项目结构

详见 [AGENTS.md](AGENTS.md) 了解项目布局和贡献者约定。

## 环境变量

| 变量 | 说明 |
|----------|------|
| `PROMPTLIB_ROOT` | 库根目录路径（覆盖默认的 `~/.promptlib`） |
| `EDITOR` | `pp edit` 和 `pp add --editor` 使用的编辑器（默认：`notepad`/`vi`） |
| `PAGER` | 长输出分页器（默认：`more`/`less -R`） |
| `NO_COLOR` | 设为任意值可禁用 ANSI 颜色 |

## 存储格式

Prompt 以人类可读的纯文本文件存储。详见 [doc/storage-format.md](doc/storage-format.md)。

## 文档

完整文档索引见 [doc/README.md](doc/README.md)。

[English](README.md)

## 许可证

MIT
