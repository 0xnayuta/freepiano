# FreePiano 现代化改造总览（2026）

更新时间：2026-03

---

# 一、项目当前状态（结论）

FreePiano 已完成核心现代化改造，当前处于：

> **工程稳定期 + 质量提升阶段**

已不再属于“架构迁移阶段”。

---

# 二、已完成工作（最终状态）

## 2.1 Unicode 全面迁移 ✅

* 全项目统一使用：

  * `UNICODE`
  * `_UNICODE`
* 全部 Win32 API 已迁移为 W 版本：

  * `CreateWindowW`
  * `MessageBoxW`
  * `AppendMenuW`
  * `OPENFILENAMEW`
* UI 层统一使用 `wchar_t` / `std::wstring`

👉 结果：

* 完全摆脱 ACP / ANSI 限制
* 支持完整 Unicode 字符集

---

## 2.2 国际化系统（i18n）重构 ✅

### 架构

* JSON 翻译文件：

  ```
  res/locales/
    ├── en.json
    ├── zh-CN.json
  ```

* 加载逻辑：

```
当前语言 → 英文 → 硬编码 fallback
```

* 核心接口：

```cpp
lang_load_string_w(id)
lang_format_string_w(...)
```

---

### 特性

* 支持 `\uXXXX` Unicode
* 支持 `\u0000`（嵌入 null）
* 支持 JSON 注释（扩展）
* 支持动态加载语言文件

---

## 2.3 运行时语言切换 ✅（重要修正）

当前系统已支持：

> **无需重启即可切换语言**

包括：

* 菜单
* 窗口标题
* 对话框文本
* 各类控件文本

---

## 2.4 语言配置持久化 ✅

配置项：

```ini
ui_language=auto | en | zh-CN
```

优先级：

```
用户配置 > 系统语言 > 默认英文
```

---

## 2.5 GUI Unicode 改造 ✅

覆盖范围：

* 主窗口
* 菜单系统
* 对话框
* 设置页（TreeView / ListView / ComboBox）
* 文件对话框
* VST 窗口

---

## 2.6 JSON 工具链 ✅

* 生成脚本：

  ```
  scripts/generate_locales.py
  ```

* 校验脚本：

  ```
  scripts/validate_locales.py
  ```

---

## 2.7 系统级回归测试 ✅

已验证：

* 启动流程
* 设置页交互
* MIDI 输入输出
* VST 插件加载
* WAV 导出
* 文件读写（含非 ASCII 路径）

👉 当前系统稳定

---

# 三、当前阶段定位

## ❗ 不再进行的工作

以下内容暂不进行：

* ❌ 重写 i18n 架构
* ❌ 替换 JSON 方案
* ❌ DSL 国际化重构
* ❌ UI 框架重构
* ❌ 大规模代码迁移（如 CMake）

---

## 🎯 当前核心目标

从“能运行”转向：

```
可观测 + 可调试 + 可维护 + 可扩展
```

---

# 四、下一阶段工作重点

## 4.1 可观测性（最高优先级）

### 目标

让系统“可被理解”，而不是黑盒。

### 需要实现

#### 日志系统

建议接口：

```cpp
fp_log_info(...)
fp_log_warn(...)
fp_log_error(...)
```

覆盖：

* i18n 加载
* JSON 解析失败
* fallback 触发
* key 缺失

输出：

* OutputDebugStringW（必须）
* 可选日志文件

---

## 4.2 i18n 调试能力

### Debug 开关

```cpp
#define I18N_DEBUG_SHOW_KEY
```

行为：

* 缺失翻译 → `[IDS_XXX]`
* fallback → 输出 warning

---

## 4.3 健壮性增强

重点改进：

* JSON 加载失败 → 自动 fallback
* 空字符串检测
* key 不存在检测
* 防止 nullptr 使用

---

## 4.4 UI 刷新机制完善

虽然已支持运行时切换语言，但需确保：

* 菜单完全刷新（含子菜单）
* 窗口标题刷新
* 所有控件刷新
* ListView / TreeView 内容刷新

建议：

```cpp
ui_refresh_all_texts()
```

---

## 4.5 开发者调试能力

建议增加：

### 启动参数

```
--lang=en
--lang=zh-CN
```

### 环境变量

```
FREEPIANO_LANG=zh-CN
```

### Debug 输出

* 当前语言
* 加载路径
* JSON 状态

---

## 4.6 工程质量改进（渐进式）

* 统一命名（lang_ / tr_）
* 减少硬编码字符串
* 清理重复逻辑
* 降低警告

---

# 五、工程规范（建议）

## 5.1 字符串使用规则

### 必须使用 i18n

* UI 文本
* 菜单
* 提示信息
* 错误信息

### 不应本地化

* 文件格式关键字
* 协议字段
* DSL 语法

---

## 5.2 新增字符串流程

1. 添加 ID
2. 更新 JSON
3. 运行校验脚本

---

## 5.3 fallback 规则

```
当前语言
   ↓
英文
   ↓
硬编码
```

---

# 六、后续中长期方向（非当前阶段）

## 可选方向

### 1. 更多语言

* 日语 / 韩语 / 繁体中文

### 2. 翻译平台

* Crowdin / Weblate

### 3. DSL 国际化整理（高风险）

* 别名分离
* 规范化输出

### 4. 构建系统升级

* CMake（仅长期考虑）

---

# 七、总结

## 当前成就

FreePiano 已完成：

* Unicode 化
* i18n 现代化
* JSON 资源体系
* 运行时语言切换
* 核心功能稳定

---

## 当前阶段核心任务

> **不是“继续设计”，而是“让系统更可靠”**

重点：

* 日志
* 调试能力
* 健壮性
* 工程规范

---

## 一句话状态

> FreePiano 已从“历史项目”升级为“可维护的现代 Win32 工程”
