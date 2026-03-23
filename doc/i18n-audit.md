# FreePiano 国际化审计与现代化改造

更新时间：2026-03-23（第三阶段完成）

---

## 已完成的全部改造

### 第一阶段：基础设施

#### 1.1 UI 语言持久化 ✅
- `config.h` / `config.cpp`：增加 `ui_language` 字段
- 语言存储为字符串代码：`auto`, `en`, `zh-CN`
- 启动时预加载用户语言配置

#### 1.2 默认语言检测现代化 ✅
- 从 `GetThreadLocale()` 迁移到 `GetUserDefaultUILanguage()`
- 支持用户配置优先

#### 1.3 宽字符接口 ✅
- `lang_load_string_w()` - 加载宽字符字符串
- `lang_format_string_w()` - 格式化宽字符字符串
- `lang_load_filter_w()` - 加载文件过滤器（支持嵌入 null）
- `lang_get_last_error_w()` - 获取宽字符错误信息
- `lang_get_code()` / `lang_from_code()` - 语言代码转换

---

### 第二阶段：GUI Unicode 迁移 ✅

#### 2.1 主窗口
- `WNDCLASSEXA` → `WNDCLASSEXW`
- `RegisterClassExA()` → `RegisterClassExW()`
- `CreateWindowA()` → `CreateWindowW()`
- 主窗口标题使用 `APP_NAME_W`

#### 2.2 主菜单
- 所有菜单文本改用 `lang_load_string_w()`
- `append_menu_text_w()` 辅助函数

#### 2.3 消息框
- 所有错误提示改用 `MessageBoxW()` + 宽字符字符串

#### 2.4 设置窗口
- TreeView 页面标题使用宽字符
- Audio/Play/MIDI/GUI 设置页所有控件使用宽字符
- ListView 列标题和内容使用宽字符
- ComboBox 下拉选项使用宽字符

#### 2.5 对话框
- `lang_localize_dialog()` 完全改用宽字符版本
- 所有对话框标题正确本地化

#### 2.6 文件对话框
- `open_dialog()` / `save_dialog()` 改用 `OPENFILENAMEW`
- 过滤器字符串正确处理嵌入 null 字符

#### 2.7 VST 窗口
- 窗口类注册和创建改用宽字符版本
- VST 效果名称正确显示

---

### 第三阶段：JSON 翻译文件支持 ✅

#### 3.1 JSON 解析器
- `json_loader.h` / `json_loader.cpp` - 无外部依赖的 JSON 解析器
- 支持 `\uXXXX` Unicode 转义
- 支持 `\u0000` 嵌入 null 字符
- 支持注释（非标准但有用）

#### 3.2 JSON 翻译文件
- `res/locales/en.json` - 165 条英语字符串
- `res/locales/zh-CN.json` - 165 条中文字符串
- `_meta` 元数据：语言代码、名称、版本

#### 3.3 代码集成
- `pch.h` 中 `#define USE_JSON_TRANSLATIONS 1` 启用
- `language.cpp` 整合 JSON 加载逻辑
- 自动回退到硬编码字符串

#### 3.4 构建集成
- 构建后自动复制 `locales/` 目录到输出
- 验证脚本 `scripts/validate_locales.py`
- 生成脚本 `scripts/generate_locales.py`

---

## 文件结构

```
freepiano/
├── res/
│   └── locales/
│       ├── en.json           # 英文翻译
│       └── zh-CN.json        # 简体中文翻译
├── src/
│   ├── language.h            # 语言接口
│   ├── language.cpp          # 语言实现（JSON + 硬编码）
│   ├── language_strdef.h     # 字符串 ID 定义 + 硬编码翻译
│   ├── json_loader.h         # JSON 解析器
│   ├── json_loader.cpp       # JSON 解析器实现
│   └── pch.h                 # USE_JSON_TRANSLATIONS 开关
├── scripts/
│   ├── generate_locales.py   # 从 language_strdef.h 生成 JSON
│   └── validate_locales.py   # 验证 JSON 完整性
└── doc/
    ├── i18n-audit.md         # 本文档
    └── i18n-json-migration.md # JSON 迁移指南
```

---

## 使用说明

### 切换 JSON / 硬编码翻译

编辑 `src/pch.h`：

```cpp
// 启用 JSON 翻译
#define USE_JSON_TRANSLATIONS 1

// 禁用 JSON 翻译（使用硬编码）
// #define USE_JSON_TRANSLATIONS 1
```

### 添加新语言

1. 创建 `res/locales/ja.json`：
```json
{
  "_meta": {
    "language": "ja",
    "name": "日本語",
    "version": "1.8"
  },
  "strings": {
    "IDS_MENU_FILE": "ファイル",
    ...
  }
}
```

2. 在 `language.h` 添加：
```cpp
#define FP_LANG_JAPANESE  2
#define FP_LANG_COUNT     3
```

3. 在 `language.cpp` 的 `lang_get_code()` 和 `lang_from_code()` 中添加映射

### 更新翻译

1. 编辑 `src/language_strdef.h` 或直接编辑 `res/locales/*.json`
2. 运行 `python scripts/generate_locales.py` 重新生成（如果修改了 .h 文件）
3. 运行 `python scripts/validate_locales.py` 验证

---

## 架构设计

### 字符串加载流程

```
lang_load_string_w(id)
    │
    ├─ USE_JSON_TRANSLATIONS=1
    │   ├─ 从 json_string_tables[当前语言] 查找
    │   ├─ 未找到 → 从 json_string_tables[英文] 查找
    │   └─ 未找到 → 回退到硬编码
    │
    └─ USE_JSON_TRANSLATIONS 未定义
        └─ 直接返回硬编码 string_table_w[语言][id]
```

### 文件对话框过滤器处理

```
"键盘配置 (*.map)\0*.map\0"
       │
       ├─ JSON 存储: "键盘配置 (*.map)\u0000*.map\u0000"
       │
       ├─ JSON 解析: std::string 包含嵌入 null
       │
       └─ 宽字符转换: utf8_to_wide_owned_len(data, size)
                      保留完整内容传递给 OPENFILENAMEW
```

---

## 辅助函数

### GUI 控件包装器

```cpp
// ListView
listview_insert_item_a/w()
listview_set_item_text_a/w()
listview_insert_column_a/w()

// TreeView
treeview_insert_item_a/w()

// ComboBox
combobox_add_string_a/w()

// Menu
append_menu_text()
append_menu_text_w()
append_menu_text_codepage()
```

---

## 验收标准 ✅

- [x] 编译成功（Unicode 工程）
- [x] 主窗口标题正常显示
- [x] 菜单文本正确本地化
- [x] 对话框标题正确本地化
- [x] 错误提示正确显示
- [x] 设置页面所有控件文本正常
- [x] 文件对话框过滤器正常工作
- [x] VST 插件窗口标题正常显示
- [x] 语言切换后重启保持设置
- [x] JSON 翻译文件正确加载
- [x] 文件类型过滤器无多余字符

---

## 后续可选改进

1. **更多语言支持**
   - 日语、韩语、繁体中文等
   - 需要社区贡献翻译

2. **RTL 语言支持**
   - 阿拉伯语、希伯来语
   - 需要布局镜像处理

3. **动态字体加载**
   - 支持更多 Unicode 音符符号
   - 支持东亚文字显示优化

4. **翻译管理平台**
   - 集成 Crowdin / Weblate
   - 自动同步翻译更新

---

## 维护指南

### 新增可本地化字符串

1. 在 `language_strdef.h` 添加：
```cpp
STR_ENGLISH  (IDS_NEW_STRING, "English text")
STR_SCHINESE (IDS_NEW_STRING, u8"中文文本")
```

2. 在 `language.h` 的枚举中添加 ID（宏会自动处理）

3. 重新生成 JSON：
```bash
python scripts/generate_locales.py
```

### 新增固定格式字段（不应本地化）

保持使用英文字符串，不要通过 `lang_load_string()` 加载：
- 文件格式关键字
- 协议字段
- 版本标识

---

## 总结

FreePiano 国际化现代化改造已完成：

| 方面 | 改造前 | 改造后 |
|------|--------|--------|
| 字符集 | ANSI/ACP | Unicode (UTF-8/UTF-16) |
| 翻译存储 | 硬编码宏 | JSON 文件（可选） |
| 语言检测 | GetThreadLocale | GetUserDefaultUILanguage |
| 语言持久化 | 无 | config 文件存储 |
| 文件对话框 | ANSI | Unicode |
| VST 窗口 | ANSI 标题乱码 | Unicode 正常显示 |

项目现在具备完整的国际化基础设施，支持社区贡献翻译，并保持向后兼容。
