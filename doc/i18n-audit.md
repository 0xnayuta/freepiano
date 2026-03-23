# FreePiano 国际化审计（第一阶段）

更新时间：2026-03-23

## 1. 当前国际化资源来源

### 1.1 UI 字符串
- `src/language_strdef.h`
- `src/language.h`
- `src/language.cpp`

说明：
- 当前核心 UI 文本由 `language_strdef.h` 中的宏表维护。
- 运行时初始化为 `string_table[语言][字符串ID]`。
- 当前仅支持：
  - `FP_LANG_ENGLISH`
  - `FP_LANG_SCHINESE`

### 1.2 对话框资源
- `res/freepiano.rc`
- `res/resource.h`

说明：
- `.rc` 中大量控件文本使用 `"IDS_..."` 占位。
- 运行时通过 `lang_localize_dialog()` 遍历窗口文本并替换成真实翻译。
- 该方案可工作，但耦合较强，后续建议迁移为“控件ID -> 字符串ID/Key”映射。

### 1.3 长文本 / 说明文本
- `res/controllers_en.txt`
- `res/controllers_ch.txt`
- `res/default_setting.txt`

说明：
- `controllers_*.txt` 通过 `TEXT` 资源 + `FindResourceEx()` 按语言加载。
- `default_setting.txt` 当前只有一份，属于默认脚本/配置内容，不按语言区分。

### 1.4 脚本 / DSL 本地化兼容层
- `src/config.cpp`

说明：
- `bind_names`
- `action_names`
- `value_action_names`
- `channel_names`

这些表同时承担：
- DSL 标准关键字
- DSL 兼容别名（含中文）
- 局部显示用途

后续建议拆分为：
- 规范语法词表
- 兼容别名字典

## 2. 当前主要问题

### 2.1 ANSI / ACP 依赖
- `src/language.cpp` 中 `utf8_to_local()`：UTF-8 -> UTF-16 -> ACP
- `src/config.cpp` 中 `utf8_to_local_name()`：UTF-8 -> UTF-16 -> ACP

问题：
- 依赖系统 ANSI 代码页
- 不利于多语言扩展
- 容易在非中文 ACP 环境中出现显示异常

### 2.2 系统语言检测过时
- 当前使用 `GetThreadLocale()`
- 后续应改为：
  - `GetUserDefaultUILanguage()`
  - 并支持用户配置优先

### 2.3 UI 语言未显式持久化
- 当前程序支持运行时切换语言
- 但启动前缺少稳定的用户语言配置入口
- 后续将增加：
  - `auto`
  - `en`
  - `zh-CN`

### 2.4 文件格式与 UI 语言边界需要明确
- `.map` 导出当前已强制英文，方向正确
- 后续需要在文档与实现中继续明确：
  - UI 可本地化
  - 存储格式保持英文规范

## 3. 当前迁移顺序（已确认）

1. 建立国际化审计文档（当前文件）
2. 为配置增加 UI 语言持久化
3. 用现代 API 替换默认语言检测
4. 为 `language` 模块增加宽字符接口
5. 再逐步推进 GUI / 菜单 / 对话框宽字符化
6. 再推进新资源格式（如 JSON）

## 4. 明确分类

### 4.1 应该本地化
- 菜单
- 对话框标题与说明
- 错误信息
- 状态文本
- 帮助说明
- 预设说明文本

### 4.2 不应随 UI 语言变化的存储格式
- `.map` 标准关键字
- 配置文件关键字
- 内部序列化格式
- 版本字段、固定协议字段

## 5. 第一阶段的验收目标

- 支持持久化 UI 语言配置
- 启动时优先使用用户语言配置
- 若为 `auto`，则使用系统 UI 语言
- 为后续 Unicode 重构补齐宽字符接口
