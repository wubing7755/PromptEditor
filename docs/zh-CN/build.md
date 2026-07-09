<!-- 对应英文版：docs/guides/environment.md + cmake.md + testing.md | 最后同步：2026-07-09 -->

# 构建与测试

## 目录

- [环境要求](#环境要求)
- [平台安装](#平台安装)
- [快速命令](#快速命令)
- [CMake 预设](#cmake-预设)
- [测试](#测试)
- [故障排除](#故障排除)


## 环境要求

| 工具 | 必须 | 说明 |
|---|---|---|
| CMake 3.21+ | 是 | 配置和生成构建文件 |
| C11 编译器 | 是 | MSVC、GCC、Clang 或 AppleClang |
| Ninja | 是（默认预设） | 构建生成器 |
| clangd | 否 | 编辑器代码导航和诊断 |

项目使用 `c_std_11` 级别的 C11 标准。支持的编译器：MSVC、GCC、Clang、AppleClang。


## 平台安装

### Windows

**推荐方案：** 安装 Visual Studio Build Tools 2022（"使用 C++ 的桌面开发"工作负载）。在"Developer PowerShell"中运行构建。使用 winget 安装 CMake 和 Ninja：

```powershell
winget install Kitware.CMake Ninja-build.Ninja
```

**备选方案 (MSYS2)：** 安装 [MSYS2](https://www.msys2.org/)，启动 UCRT64 shell：

```sh
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-{cmake,ninja,gcc}
```

### Linux

| 发行版 | 安装命令 |
|---|---|
| Debian / Ubuntu | `sudo apt install cmake ninja-build gcc` |
| Fedora / RHEL | `sudo dnf install cmake ninja-build gcc` |
| Arch Linux | `sudo pacman -S cmake ninja gcc` |

### macOS

```sh
xcode-select --install
brew install cmake ninja
```

### 验证

```sh
./scripts/bootstrap.sh        # Windows：.\scripts\bootstrap.ps1
```


## 快速命令

```sh
# 完整检查（配置、构建、测试）
./scripts/check.sh      # Windows：.\scripts\check.ps1

# 逐步执行
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

详细的 CMake 工作流说明见 [../guides/cmake.md](../guides/cmake.md)。


## CMake 预设

| 预设 | 用途 |
|---|---|
| `ninja-debug` | 默认跨平台开发构建（带调试符号 + compile_commands.json） |
| `ninja-release` | 优化发布构建 |
| `ninja-shared` | 动态库构建 |
| `ninja-asan` | AddressSanitizer + UBSan |
| `ninja-coverage` | 覆盖率构建 |
| `debug` / `release` | 生成器无关兼容预设 |

预设通过 `CMakePresets.json` 统一管理。创建本地预设请复制 `CMakeUserPresets.example.json` 为 `CMakeUserPresets.json` 后编辑。


## 测试

使用 CTest 运行自动化测试：

```sh
ctest --preset ninja-debug --output-on-failure
```

### 测试要求

- Bug 修复应包含回归测试
- 新公共 API 应包含成功和失败用例
- 数值 API 应覆盖边界和溢出情况

### 静态分析

CI 运行 clang-format、clang-tidy、cppcheck 和 YAML 检查：

```sh
./scripts/check.sh --enable-tidy      # Linux/macOS
./scripts/check.ps1 -EnableTidy       # Windows
```

### 测试分类

| 改动类型 | 建议检查 |
|---|---|
| C 源码或公共头文件 | `./scripts/check.ps1` 或 `./scripts/check.sh` |
| 动态库/导出宏变更 | `ninja-shared` 预设 + 共享库包冒烟测试 |
| 安装/包元数据变更 | 静态和共享包冒烟测试 |
| CI 或 YAML 变更 | `git diff --check` + CI 静态分析 |
| 发布敏感变更 | `ninja-release`、`ninja-shared`、`ninja-asan` + 包冒烟 |


## 故障排除

**CMake 缓存指向错误路径：**
- 现象：configure 提示不同的源目录
- 解决：删除对应构建目录后重新配置

**找不到 Ninja 或 C 编译器：**
- 现象：configure 提示找不到 Ninja 或 `CMAKE_C_COMPILER`
- 解决：运行 `./scripts/bootstrap.sh` 获取诊断。Windows 需在 Developer PowerShell 中操作

**VS Code 使用 Ninja 构建 `ALL_BUILD` 失败：**
- 解决：运行 `CMake: Set Build Target` 选择 `all`

**找不到 `find_package(PromptLib)`：**
- 解决：先安装项目，传递 `-DCMAKE_PREFIX_PATH=/path/to/install`

**Windows 动态库测试找不到 DLL：**
- 解决：将 DLL 复制到可执行文件旁边或将安装 `bin` 目录加入 `PATH`
