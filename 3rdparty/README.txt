# 第三方库集成说明

Prism 项目依赖以下第三方库，请将它们克隆到 `3rdparty/` 目录下。

## 必需库

### 1. ElaWidgetTools
- **仓库地址**: https://github.com/Liniyous/ElaWidgetTools
- **版本要求**: 最新版本
- **安装命令**:
  ```bash
  cd 3rdparty
  git clone https://github.com/Liniyous/ElaWidgetTools.git
  ```

### 2. yaml-cpp
- **仓库地址**: https://github.com/jbeder/yaml-cpp
- **版本要求**: 0.7.0+
- **安装命令**:
  ```bash
  cd 3rdparty
  git clone https://github.com/jbeder/yaml-cpp.git
  ```

## 完整安装脚本

### Windows (PowerShell)
```powershell
cd 3rdparty
git clone https://github.com/Liniyous/ElaWidgetTools.git
git clone https://github.com/jbeder/yaml-cpp.git
```

### Linux/macOS (Bash)
```bash
cd 3rdparty
git clone https://github.com/Liniyous/ElaWidgetTools.git
git clone https://github.com/jbeder/yaml-cpp.git
```

## 构建说明

安装完第三方库后，返回项目根目录执行：

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 目录结构验证

完成后，`3rdparty/` 目录结构应该如下：

```
3rdparty/
├── CMakeLists.txt
├── ElaWidgetTools/
│   ├── CMakeLists.txt
│   └── ...
└── yaml-cpp/
    ├── CMakeLists.txt
    └── ...
```
