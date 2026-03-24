# JSON 翻译文件迁移指南

## 概述

本指南说明如何将硬编码的翻译字符串从 `language_strdef.h` 迁移到 JSON 文件格式。

## 文件结构

```
freepiano/
├── res/
│   └── locales/
│       ├── en.json        # 英文翻译
│       ├── zh-CN.json     # 简体中文翻译
│       └── ...            # 其他语言
├── src/
│   ├── language.h         # 语言接口
│   ├── language.cpp       # 语言实现
│   ├── json_loader.h      # JSON 解析器
│   └── json_loader.cpp    # JSON 解析器实现
└── doc/
    └── i18n-json-migration.md
```

## JSON 格式

```json
{
  "_meta": {
    "language": "zh-CN",
    "name": "简体中文",
    "version": "1.8",
    "author": "Contributor Name"
  },
  "strings": {
    "IDS_MENU_FILE": "文件",
    "IDS_MENU_HELP": "帮助",
    "IDS_ERR_LOAD_VST": "无法载入VST插件，错误码：%d"
  }
}
```

### 特殊字符处理

| 字符 | JSON 表示 |
|------|-----------|
| `\0`（嵌入 null） | `\u0000` |
| `\n`（换行） | `\n` |
| `"`（引号） | `\"` |
| `\`（反斜杠） | `\\` |

## 迁移步骤

### 步骤 1：添加 JSON 解析器到项目

编辑 `vc/freepiano.vcxproj`：

```xml
<ClCompile Include="..\src\json_loader.cpp" />
<ClInclude Include="..\src\json_loader.h" />
```

### 步骤 2：修改 language.cpp

在 `lang_init()` 函数中添加 JSON 加载逻辑：

```cpp
void lang_init() {
  // Initialize JSON-based translations first
  #ifdef USE_JSON_TRANSLATIONS
    lang_init_from_json();
  #else
    // Fallback: use hardcoded strings from language_strdef.h
    // ... existing code ...
  #endif
}
```

### 步骤 3：定义 USE_JSON_TRANSLATIONS

方式 A：在 `pch.h` 中添加：
```cpp
#define USE_JSON_TRANSLATIONS 1
```

方式 B：在项目设置中添加预处理器定义：
- C/C++ → Preprocessor → Preprocessor Definitions
- 添加 `USE_JSON_TRANSLATIONS`

### 步骤 4：部署 JSON 文件

将 `res/locales/*.json` 文件复制到输出目录：
- 与 `freepiano.exe` 同级目录下的 `locales/` 文件夹

## 编译选项

### 保持硬编码（当前状态）
- 不定义 `USE_JSON_TRANSLATIONS`
- 使用 `language_strdef.h` 中的字符串

### 使用 JSON 文件
- 定义 `USE_JSON_TRANSLATIONS`
- 从 `locales/` 目录加载 JSON 文件

## 运行时行为

```
启动流程：
1. lang_init() 被调用
2. 如果 USE_JSON_TRANSLATIONS：
   a. 扫描 locales/ 目录
   b. 加载所有 .json 文件
   c. 按 _meta.language 确定 ID
3. 否则使用硬编码字符串

字符串查找：
1. 查找当前语言的 JSON 表
2. 如果未找到，回退到英文
3. 如果仍未找到，返回空字符串
```

## 添加新语言

1. 创建 `res/locales/ja.json`：
```json
{
  "_meta": {
    "language": "ja",
    "name": "日本語"
  },
  "strings": {
    "IDS_MENU_FILE": "ファイル",
    "IDS_MENU_HELP": "ヘルプ",
    ...
  }
}
```

2. 在 `language.h` 添加：
```cpp
#define FP_LANG_JAPANESE  2
#define FP_LANG_COUNT     3
```

3. 在 `language.cpp` 添加：
```cpp
case FP_LANG_JAPANESE: return "ja";
case FP_LANG_JAPANESE: return MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
```

## 社区贡献

JSON 格式便于社区贡献翻译：

1. 贡献者 Fork 项目
2. 复制 `en.json` 为 `xx.json`
3. 翻译 `strings` 下的所有条目
4. 提交 Pull Request

### 验证脚本

创建 `scripts/validate_locales.py`：

```python
#!/usr/bin/env python3
import json
import os
import sys

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)

def validate_locales():
    base = 'res/locales'
    en = load_json(os.path.join(base, 'en.json'))
    en_keys = set(en['strings'].keys())

    errors = []
    for filename in os.listdir(base):
        if not filename.endswith('.json'):
            continue

        path = os.path.join(base, filename)
        data = load_json(path)
        lang_keys = set(data['strings'].keys())

        missing = en_keys - lang_keys
        extra = lang_keys - en_keys

        if missing:
            errors.append(f"{filename}: Missing keys: {missing}")
        if extra:
            errors.append(f"{filename}: Unknown keys: {extra}")

    if errors:
        for e in errors:
            print(e, file=sys.stderr)
        sys.exit(1)
    else:
        print("All locale files are valid!")

if __name__ == '__main__':
    validate_locales()
```

## 性能考虑

### 内存占用
- JSON 解析器一次加载所有字符串到内存
- 约 50KB / 语言（当前翻译量）

### 启动时间
- JSON 解析增加约 10-20ms
- 可忽略不计

### 替代方案：嵌入资源

如果希望避免外部文件，可将 JSON 嵌入到 PE 资源：

1. 修改 `freepiano.rc`：
```
LOCALE_EN    TEXT    "locales/en.json"
LOCALE_ZH_CN TEXT    "locales/zh-CN.json"
```

2. 使用 `FindResource()` + `LoadResource()` 加载

## 回滚策略

如果遇到问题，可以立即回滚：

1. 移除 `USE_JSON_TRANSLATIONS` 定义
2. 重新编译

原有的 `language_strdef.h` 机制仍然存在。

## 总结

| 方面 | 硬编码 | JSON 文件 |
|------|--------|-----------|
| **修改翻译** | 重新编译 | 直接编辑 JSON |
| **添加语言** | 修改代码 | 添加文件 |
| **社区贡献** | 困难 | 简单 |
| **部署** | 单文件 | 需要目录 |
| **启动速度** | 最快 | 稍慢（~10ms） |

**推荐**：对于需要支持社区翻译的项目，使用 JSON 文件。对于只支持固定语言的内部项目，保持硬编码即可。
