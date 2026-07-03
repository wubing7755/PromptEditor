# PromptEditor - 纯 C Prompt 管理 CLI

> [English](README.md)

在本地文件夹中保存、组织、搜索、浏览和优化 prompt 模板 - 纯 C 实现，零外部依赖。

## 特性

- **存储 prompt** 在本地文件夹中 - 纯文本文件，无需数据库
- **组织管理** 按文件夹、分类和标签
- **交互式搜索** 使用 `pl browse`（如有 `fzf` 则使用模糊搜索，否则使用编号菜单）
- **编辑器编辑** 使用 `pl edit <id>` 在 `$EDITOR` 中编辑 prompt 内容
- **版本管理** 使用 `pl optimize` 创建优化版本，支持历史对比和版本晋升
- **导出/导入/备份** 完整库操作
- **JSON 输出** 使用 `--json` 标志，适合脚本处理
- **彩色终端输出** 自动分页（遵循 `NO_COLOR` 规范）
- **跨平台** - Windows (MinGW/MSVC)、Linux、macOS

## 快速开始

```powershell
# Windows PowerShell
git clone <repo>
cd PromptEditor
./scripts/bootstrap.ps1
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

```sh
# Linux/macOS
git clone <repo>
cd PromptEditor
./scripts/bootstrap.sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

初始化库并保存第一个 prompt：

```sh
./build/ninja-debug/bin/prompteditor init
./build/ninja-debug/bin/prompteditor add --title "Hello" --body "这是我的第一个 prompt。"
./build/ninja-debug/bin/prompteditor list
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
# 初始化库
promptlib init --root ./my-prompts

# 添加 prompt（打开 $EDITOR 编辑内容）
promptlib add --title "Summarize" --editor --root ./my-prompts

# 使用内联内容添加
promptlib add --title "Translate" --body "请将这段文字翻译成中文。"   --folder translations --category utility --tag language   --root ./my-prompts

# 列出所有 prompt（彩色表格，自动分页）
promptlib list --root ./my-prompts

# 以 JSON 格式列出
promptlib list --root ./my-prompts --json

# 显示 prompt
promptlib show --root ./my-prompts "translate"

# 仅显示原始内容（适合管道处理）
promptlib show --root ./my-prompts "translate" --raw

# 在编辑器中编辑内容
promptlib edit --root ./my-prompts "translate"

# 编辑特定字段
promptlib edit --root ./my-prompts "translate" --title "Renamed" --category advanced

# 搜索
promptlib search --root ./my-prompts "Chinese" --json

# 交互式浏览
promptlib browse --root ./my-prompts

# 带版本管理的优化
promptlib optimize --root ./my-prompts "translate"   --body "请将提供的文本翻译成中文。" --note "改进措辞"

# 查看版本历史
promptlib optimize --root ./my-prompts "translate" --history

# 晋升优化版本为当前版本
promptlib optimize --root ./my-prompts "translate" --promote

# 导出/导入/备份
promptlib export --root ./my-prompts --out ./backup
promptlib import ./backup --root ./restored

# 删除（默认归档）
promptlib delete --root ./my-prompts "translate" --yes
```

## 构建与测试

### CMake Presets

项目使用 CMake presets 管理常见构建流程。运行 bootstrap 后：

| Preset | 说明 |
|--------|------|
| `ninja-debug` | Ninja Debug 构建（默认） |
| `ninja-release` | 优化 Release 构建 |
| `ninja-shared` | Debug 共享库构建 |
| `ninja-asan` | 带 AddressSanitizer + UBSan 的 Debug 构建 |
| `ninja-coverage` | 带覆盖率标志的 Debug 构建 |

### 快速检查

```powershell
# Windows PowerShell
./scripts/check.ps1

# 带静态分析
./scripts/check.ps1 -EnableTidy
```

```sh
# Linux/macOS
./scripts/check.sh
./scripts/check.sh --enable-tidy
```

### 手动构建

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

### 支持的编译器

- GCC 8+（主要，在 MinGW 和 Linux 上测试）
- Clang 7+（通过 `-DCMAKE_C_COMPILER=clang`）
- MSVC 2019+（通过 Visual Studio 生成器）

## 项目结构

```
include/prompteditor/    公开头文件
  cli.h                  CLI 入口
  compiler.h             编译器/平台检测
  export.h               DLL 导出宏
src/                     实现代码
  cli.c                  CLI 实现
  prompteditor/          核心库源码
  main.c                 可执行文件入口
tests/                   CTest 测试程序
  test_example.c         核心库测试
  package_smoke/         下游包 smoke 测试
  subproject_smoke/      add_subdirectory() smoke 测试
  support/               测试断言辅助
cmake/                   CMake 构建模块
scripts/                 本地 bootstrap 和检查脚本
doc/                     文档
  mvp.md                 MVP 功能跟踪
  storage-format.md      库存储布局规范
  guides/                主题指南（构建、测试、发布等）
  adr/                   架构决策记录
```

## 环境变量

| 变量 | 说明 |
|----------|------|
| `PROMPTLIB_ROOT` | 库根目录路径（覆盖默认的 `~/.promptlib`） |
| `EDITOR` | `pl edit` 和 `pl add --editor` 使用的编辑器（默认：`notepad`/`vi`） |
| `PAGER` | 长输出分页器（默认：`more`/`less -R`） |
| `NO_COLOR` | 设为任意值可禁用 ANSI 颜色 |

## 存储格式

Prompt 以人类可读的纯文本文件存储：

```
<library-root>/
  .promptlib/               内部元数据
    index.tsv               可重建的 prompt 索引
    folders.tsv             已注册的文件夹
    categories.tsv          已注册的分类
  prompts/                  Prompt 内容
    <folder>/
      <prompt-id>/
        current.txt         当前 prompt 正文
        metadata.tsv        ID、标题、文件夹、分类、标签、时间戳
        versions/           优化版本
          index.tsv         版本历史
          0002.txt          0002.tsv
  archive/                  已删除的 prompt
```

详细说明请见 [doc/storage-format.md](doc/storage-format.md)。

## 文档

- [MVP 功能跟踪](doc/mvp.md) - 已实现和计划中的命令
- [存储格式](doc/storage-format.md) - 库布局和文件格式
- [构建指南](doc/guides/cmake.md) - CMake presets、生成器、故障排除
- [测试指南](doc/guides/testing.md) - CTest、静态分析、smoke 测试
- [环境配置](doc/guides/environment.md) - 平台前置条件
- [发布指南](doc/guides/release.md) - 版本管理和打包
- [贡献指南](CONTRIBUTING.md) - 如何贡献
- [安全策略](SECURITY.md) - 安全策略

[English](README.md)

## 许可证

MIT
