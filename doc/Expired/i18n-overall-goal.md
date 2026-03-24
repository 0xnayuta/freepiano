# 总体目标

把当前国际化从：

- ANSI/MBCS 风格
- 自定义混合式文本来源
- 只适合中英两种语言
- 运行时替换脆弱

逐步升级为：

- Unicode-first
- 语言资源统一管理
- UI 文本、长文本、脚本显示分层
- 用户语言可持久化
- 便于以后扩展更多语言

---

## 一、推荐路线图

我建议分成 4 个阶段。

---

### Phase 0：先做“可观测”和“止血”

目标：不大改架构，先把问题摸清并降低乱码风险。

#### 0.1 建立国际化清单

整理当前所有文本来源：

1. src/language_strdef.h
2. res/freepiano.rc
3. res/controllers_en.txt
4. res/controllers_ch.txt
5. res/default_setting.txt
6. src/config.cpp 里的 DSL 名称表
7. 代码里硬编码字符串

输出一个表格，字段建议：

- 文本ID/用途
- 当前来源文件
- 是否用户可见
- 是否需翻译
- 是否属于 DSL 关键字
- 是否必须保持英文规范
- 编码现状
- 风险级别

#### 0.2 扫描硬编码字符串

重点找：

- MessageBox(...)
- SetWindowText(...)
- AppendMenu(...)
- ShellExecute(...) 关联标题文本
- printf/sprintf 用于 UI 的文本
- 任何中文/英文直接写死的位置

目标是先知道还有多少文本没纳入统一管理。

#### 0.3 明确哪些“不能本地化”

特别是脚本系统要分层：

##### 应该本地化

- UI标签
- 菜单
- 对话框说明
- 错误提示
- 帮助说明
- 控制器预设描述文本

##### 不建议本地化为存储格式

- .map 文件标准关键字
- 配置文件键名
- 内部协议字段
- 序列化格式

这一步非常重要，否则后面容易把“显示语言”和“文件格式语言”混在一起。

---

## 二、Phase 1：低风险现代化修复

目标：不改变功能逻辑，先把底层编码和语言选择修正到现代可维护状态。

### 1.1 全项目转为 Unicode 构建

这是第一优先级。

#### 目标

Visual Studio 工程统一使用：

- UNICODE
- _UNICODE

并尽量走 Win32 宽字符 API。

#### 要改的方向

- MessageBox -> MessageBoxW
- SetWindowText -> SetWindowTextW
- GetWindowText -> GetWindowTextW
- AppendMenu -> AppendMenuW
- GetOpenFileName / GetSaveFileName -> ...W
- 文件路径尽量用宽字符版本 Win32 API

#### 原因

这一步做完后，国际化才有真正的地基。否则你后面无论资源怎么整理，都会被 CP_ACP 拖住。

---

### 1.2 废弃 UTF-8 -> ACP，改为内部 UTF-16 或 UTF-8 明确策略

当前的 utf8_to_local() 是历史包袱，建议废弃。

#### 推荐方案 A：Win32 UI 层统一 std::wstring

这是最适合当前项目的。

- 源文本资源可继续用 UTF-8 文件存储
- 加载时转成 UTF-16
- UI 层只使用 wstring / wchar_t*
- Win32 API 全走 W 版本

#### 推荐方案 B：内部 UTF-8，Win32 边界转 UTF-16

也可以，但对于老 Win32 项目，维护成本通常比 A 高。

#### 我建议

UI 与资源层统一成 UTF-16。因为这是原生 Win32 最顺手的现代化方案。

---

### 1.3 替换语言检测方式

当前 GetThreadLocale() 建议替换。

#### 新逻辑建议

优先级：

1. 用户配置里保存的语言
2. GetUserDefaultUILanguage() / GetThreadUILanguage()
3. 默认英文

未来如果要更完整，可支持：

- GetUserPreferredUILanguages()

#### 语言模型建议

不要再用纯整数常量作为最终模型，改为语言标签：

- en
- zh-CN

内部仍可映射为 enum，但外部配置和资源索引最好用字符串标签。

---

### 1.4 把“当前语言”持久化到配置

当前看起来语言切换可能不会保存。

建议在配置中新增：

```ini
ui_language=auto
```

或

```ini
ui_language=en
ui_language=zh-CN
```

#### 行为建议

- 默认 auto
- 用户手动切换后保存
- 下次启动优先使用配置值

这属于小改动、大体验提升。

---

### 1.5 建立统一字符串访问接口

保留 lang_load_string() 的思路，但升级接口。

建议未来目标接口类似：

```cpp
std::wstring tr(StringId id);
std::wstring trf(StringId id, ...);
std::vector<std::wstring> tr_list(StringId id);
```

把现在这些能力统一起来：

- 普通字符串
- 格式化字符串
- 逗号分隔列表
- 长文本资源

避免继续散落成：

- lang_load_string
- lang_load_string_array
- lang_text_open
- 直接 LoadString
- 直接资源读取

---

## 三、Phase 2：资源结构收敛

目标：把“混合式国际化”收敛成清晰结构。

### 2.1 统一资源来源：拆成 3 类

我建议未来只保留三类国际化资源：

#### A. UI字符串

例如：
- 菜单
- 按钮
- 提示
- 错误信息
- 状态栏文本

#### B. 长文本/帮助文本

例如：
- 控制器说明
- 预设说明
- 帮助页面

#### C. DSL显示别名

例如：
- 给用户看的脚本关键字说明
- 解析时兼容的旧中文别名

这三类不要混用。

---

### 2.2 用独立语言资源文件替代 language_strdef.h 宏表

当前宏表能用，但不现代。

#### 推荐资源格式

优先推荐：

- JSON
- YAML
- INI 也可，但不够强
- 如果想更标准，也可以 .po / gettext，但对 Win32 老项目未必最省事

#### 我建议

先简单落地，使用：

- res/i18n/en.json
- res/i18n/zh-CN.json

例如：

```json
{
  "menu.file": "File",
  "menu.file.open": "Open...",
  "menu.file.save": "Save...",
  "error.open_song": "Failed to open song."
}
```

#### 优势

- 翻译不需要改 C++ 头文件
- 容易校验 key 缺失
- 容易做脚本检查
- 以后加语言很简单

#### 兼容策略

第一阶段不用立刻删掉 language_strdef.h，可以：

- 新增 JSON loader
- 先与旧表并存
- 最后切换

---

### 2.3 对话框不要再靠“控件文本等于 ID 名称”替换

这是当前最脆弱的一块之一。

#### 推荐做法

对每个对话框维护一个“控件ID -> 字符串key”的映射表。

例如：

```cpp
localize_control(hWnd, IDC_SOMETHING, L"settings.play.velocity");
localize_window(hWnd, L"settings.caption");
```

或者做成一个数组：

```cpp
static const LocalizedControl kSettingsControls[] = {
  {0, L"settings.caption", Target::WindowTitle},
  {IDC_PLAY_VELOCITY, L"settings.play.velocity", Target::Text},
};
```

#### 好处

- 不依赖 RC 文本内容
- 更可维护
- 更适合自动检查
- 更适合后续支持 tooltip、column、tab 页标题等复杂场景

---

### 2.4 菜单定义从“文本硬编码创建”升级为“结构 + key”

当前菜单结构在代码里直接写：

```cpp
AppendMenu(..., lang_load_string(...));
```

建议改成：

- 菜单结构代码保留
- 文本 key 独立

甚至可以进一步定义菜单描述表。

这样以后切换语言时只需要：

- 重建菜单
- 从统一资源里取文本

---

### 2.5 长文本资源也改成外部文本文件

当前 controllers_en.txt / controllers_ch.txt 通过 Win32 TEXT resource 按语言加载，能用，但不够现代。

建议改成：

- res/i18n/en/controllers.txt
- res/i18n/zh-CN/controllers.txt

或者直接放进 JSON 的长文本字段。

#### 好处

- 更容易编辑
- 不依赖 RC 编译
- 更利于翻译协作
- 更容易热更新或热加载

如果你希望“单 exe 打包”，后面也可以再嵌回资源，但逻辑层别再绑定 FindResourceEx。

---

## 四、Phase 3：脚本系统国际化重构

目标：保留兼容性，但让 DSL 规则更清晰。

### 3.1 保持文件格式“英文规范”

这个一定要坚持。

#### 建议规则

- .map 保存时始终写英文 DSL
- 内部序列化和导出统一英文
- 文档中把英文 DSL 作为标准语法

#### 可以保留的兼容

解析器继续兼容旧中文关键字，避免历史文件失效：

- 键盘按下 -> Keydown
- 曲调 -> KeySignature

但这是兼容层，不是主存储格式。

---

### 3.2 把 DSL 解析别名从 UI 文案里独立出来

当前 config.cpp 把很多中文别名直接混在解析表里。

建议分成两层：

#### 语法标准名

```
Keydown
Keyup
Label
Color
```

#### 本地化别名表

```json
"dsl.alias.Keydown": ["Keydown", "键盘按下"]
```

或者保留 C++ 静态表，但明确标记这是“兼容别名”，不是 UI 翻译源。

---

### 3.3 增加 DSL 版本化与规范化工具

建议增加一个内部工具函数：

- 读取旧 .map
- 输出规范化英文 .map

这样以后可以：

- 自动迁移历史脚本
- 调试解析器
- 减少多语言别名带来的维护成本

---

## 五、Phase 4：更现代的工程化能力

这部分不是必须第一时间做，但很值。

### 4.1 增加 i18n 校验脚本

建议加一个小工具或构建脚本，检查：

- 所有 key 是否在各语言中齐全
- 是否有未使用 key
- 是否有重复 key
- 格式化占位符是否一致
- 比如 %s, %d 数量一致
