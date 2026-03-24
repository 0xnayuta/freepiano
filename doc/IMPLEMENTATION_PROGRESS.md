# FreePiano 阶段性实现进展（2026-03）

更新时间：2026-03-24

---

# 一、本轮目标

本轮工作基于 `doc/Prompt/prompt_1.md` 推进，目标不是重构架构，而是在现有 Unicode / JSON i18n 基础上，补齐以下能力：

* 可观测性（日志）
* i18n 调试能力
* i18n 健壮性
* 启动期语言覆盖能力
* 运行时语言切换后的统一刷新入口

---

# 二、本轮已完成内容

## 2.1 轻量日志系统已落地 ✅

新增文件：

* `src/fp_log.h`
* `src/fp_log.cpp`

已实现接口：

```cpp
fp_log_info(const wchar_t* fmt, ...)
fp_log_warn(const wchar_t* fmt, ...)
fp_log_error(const wchar_t* fmt, ...)
```

当前输出位置：

* `OutputDebugStringW`
* `logs/freepiano.log`

日志文件位置：

```txt
<程序目录>\logs\freepiano.log
```

---

## 2.2 日志已接入关键链路 ✅

已接入模块：

* `src/main.cpp`
* `src/config.cpp`
* `src/language.cpp`
* `src/json_loader.cpp`
* `src/gui.cpp`
* `src/display.cpp`
* `src/output_wasapi.cpp`

当前已覆盖的典型场景：

### 启动流程

* 启动开始
* COM 初始化成功/失败
* GUI 初始化成功/失败
* Display 初始化成功/失败
* Keyboard 初始化成功/失败
* 主窗口显示
* 消息循环退出
* 程序关闭完成

### 配置流程

* 配置子系统初始化
* 配置预加载语言
* 配置文件不存在时使用默认值
* 配置加载开始/结束
* 配置保存开始/成功/失败

### i18n / JSON

* locale JSON 加载成功/失败
* 英文 locale 缺失导致 JSON 翻译关闭
* JSON 打开失败
* JSON 文件为空
* JSON 读取失败
* JSON 解析失败
* fallback 触发
* key 缺失
* 非法 UTF-8
* 空 key / 空 value 检查

---

## 2.3 旧 stderr 输出已进行一轮收口 ✅

已替换掉主链路中较关键的一批：

* 主窗口创建失败
* 字体加载失败 / 字体缺失
* WASAPI 初始化关键失败点

说明：

* 目前仍可能存在个别非关键历史输出
* 但主链路已基本转入 `fp_log_*`

---

## 2.4 i18n 健壮性已增强 ✅

增强位置：

* `src/language.cpp`

当前已补充：

### 防御性处理

* 非法字符串 ID 检查
* `nullptr` 防护
* 格式化函数参数保护：
  * `buff == NULL`
  * `size == 0`
* `lang_set_last_error()` 的空格式串保护

### 编码安全

* UTF-8 → UTF-16 转换启用 `MB_ERR_INVALID_CHARS`
* 非法 UTF-8 不再静默吞掉
* 出错后进入 fallback 并记录日志

### JSON 数据审计

* 空 key 检测
* 空 value 检测

### fallback 更明确

当前字符串获取逻辑为：

```txt
当前语言 JSON
  ↓
英文 JSON
  ↓
硬编码字符串
```

并伴随：

* fallback warning 日志
* missing key error 日志
* invalid UTF-8 error 日志

为避免刷屏，相关日志做了去重记录。

---

## 2.5 i18n 调试开关已实现 ✅

更新位置：

* `src/pch.h`
* `src/language.cpp`

当前支持两档调试模式。

### 模式 A：最终缺失显示 key

```cpp
#define I18N_DEBUG_SHOW_KEY 1
```

行为：

* 当字符串在当前语言、英文、硬编码中都不可用时
* 显示：

```txt
[IDS_XXX]
```

适合：

* 排查最终真正缺字

---

### 模式 B：fallback 即显示调试标记

```cpp
#define I18N_DEBUG_SHOW_FALLBACK 1
```

行为：

* 只要当前语言没有命中、发生 fallback
* 直接显示：

```txt
[IDS_XXX]!
```

适合：

* 快速检查当前语言覆盖率
* 找出仍依赖英文/硬编码 fallback 的字符串

---

### 说明

* 文件过滤器字符串未启用激进 fallback 调试显示
* 以避免破坏 `OPENFILENAMEW` 所需的双 null 过滤器结构

---

## 2.6 启动期语言覆盖能力已实现 ✅

实现位置：

* `src/main.cpp`

支持：

### 环境变量

```txt
FREEPIANO_LANG=en
FREEPIANO_LANG=zh-CN
```

### 启动参数

```txt
--lang=en
--lang=zh-CN
--lang en
--lang zh-CN
```

当前优先级：

```txt
命令行参数 > 环境变量 > 配置文件 > 系统语言 > 默认英文
```

说明：

* 启动覆盖仅影响本次运行
* 不会把临时调试语言写回配置文件
* 适合开发测试与截图验证

---

## 2.7 统一 UI 刷新入口已建立 ✅

实现位置：

* `src/gui.h`
* `src/gui.cpp`

新增接口：

```cpp
gui_refresh_all_texts()
```

当前已接入：

* `gui_set_language(int lang)`

当前刷新行为包括：

* 主窗口标题刷新
* 主菜单重建
* `DrawMenuBar()` 刷新菜单栏
* 主窗口重绘
* `display_force_refresh()` 刷新显示层
* 设置窗口打开时关闭并重建入口收口
* 键位设置窗口打开时关闭并重建入口收口

说明：

* 当前策略仍偏保守：对复杂窗口优先关闭重建，而不是强行原地刷新
* 这是当前阶段低风险、可控的实现方式

---

# 三、本轮修改涉及的主要文件

## 新增文件

* `src/fp_log.h`
* `src/fp_log.cpp`
* `doc/IMPLEMENTATION_PROGRESS.md`
* `doc/VALIDATION_CHECKLIST.md`
* `doc/PATH_CLASSIFICATION.md`
* `doc/HANDOFF_GUIDE.md`
* `doc/CHANGELOG_2026-03.md`

## 主要修改文件

* `src/main.cpp`
* `src/config.cpp`
* `src/language.cpp`
* `src/json_loader.cpp`
* `src/gui.cpp`
* `src/gui.h`
* `src/display.cpp`
* `src/output_wasapi.cpp`
* `src/pch.h`
* `vc/freepiano.vcxproj`
* `doc/PROJECT_STATUS.md`

---

# 四、当前项目状态更新

经过本轮改动，项目状态可以更明确地表述为：

> FreePiano 已完成核心 Unicode / JSON i18n 现代化改造，并已进入“可观测性、调试能力、健壮性、运行时语言切换体验”补强阶段。

相较于本轮之前，当前项目新增了：

* 统一日志能力
* i18n 缺失/回退可观测性
* 调试显示开关
* 启动时强制语言能力
* 统一 UI 刷新入口
* ANSI / ACP 遗留点审计与边界收敛策略

---

# 五、仍未完成或可继续改进的部分

## 5.1 UI 刷新仍可进一步细化

当前已有统一入口，但仍以“复杂窗口关闭重建”为主。

后续可继续考虑：

* 设置窗口当前页原地重建
* TreeView / ListView / ComboBox 更细粒度刷新
* 更多 modeless 窗口纳入统一刷新管理

---

## 5.2 历史 ANSI / ACP 路径尚未完全收口

虽然主 UI Unicode 链路已完成，但仍存在：

* `CP_ACP` 转换路径
* 少量第三方/历史兼容边界
* 部分 legacy-only 接口

当前已完成的收口包括：

* 文件对话框路径转换入口集中封装
* 菜单不再回退到 `AppendMenuA`
* `gui.cpp` 中用户可见文本的直接 A 版读取/写入已基本清空
* `lang_load_string_array()` 已明确标记为 legacy-only 兼容接口，且当前无直接调用点
* `config` / `ASIO` / `VST` / 注册表读取等边界点已补充失败处理、日志、注释说明，并进一步集中到少数辅助转换函数

当前仍保留但已被明确标记的兼容边界，主要集中在：

* `config.cpp` 的窄字符名表兼容链路
* `output_asio.cpp` 的驱动/设备名边界
* `synthesizer_vst.cpp` 的插件名边界
* `asio/asiolist.cpp` 的注册表 CLSID 文本边界

这类问题不应一次性重构，应继续渐进式清理。

---

## 5.3 其他模块日志仍可继续统一

目前主链路已基本接入，但仍可以继续清理：

* 非主路径模块的历史输出
* 更细粒度的设备/插件调试信息

---

## 5.4 自动化验证仍可增强

当前已有 locale 校验脚本，但后续仍可增加：

* 更明确的调试验证清单
* 语言切换专项检查清单
* fallback / missing key 回归检查

---

# 六、建议的下一步方向

建议继续按“小步、低风险、可验证”的方式推进，优先级如下：

## 优先级 1

* 继续补齐 UI 原地刷新能力
* 把更多窗口纳入 `gui_refresh_all_texts()` 管理

## 优先级 2

* 继续收口剩余历史 `stderr` / ANSI 输出
* 扩展日志覆盖面
* 按 `doc/ANSI_ACP_AUDIT.md` 继续分批治理剩余兼容边界
* 在“可直接替换”为 W/Unicode 的点与“暂时保留的兼容边界（legacy boundary）”之间保持明确分层

## 优先级 3

* 持续维护 `doc/VALIDATION_CHECKLIST.md`
* 优先执行其中的“建议优先执行的最小验证集”
* 对 i18n 增加更明确的验证清单与测试说明

---

# 七、一句话总结

> 本轮工作已把 FreePiano 从“具备 i18n 能力”推进到“具备 i18n 调试、日志观测、启动覆盖、统一刷新入口和兼容边界审计”的阶段，后续重点应继续放在增量补强，而不是架构重写。

---

# 八、阶段结语（便于后续接手）

## 8.1 当前阶段已完成的关键事项

本阶段已经完成的高价值工作可以概括为：

* 日志系统已落地，并接入启动、配置、i18n、JSON、GUI、WASAPI 等主链路
* JSON i18n 主流程已具备缺失 key、fallback、非法 UTF-8 的可观测性
* 已支持运行时语言切换，并建立统一刷新入口 `gui_refresh_all_texts()`
* 已支持启动期语言覆盖：
  * `FREEPIANO_LANG`
  * `--lang=en`
  * `--lang=zh-CN`
* 已实现 i18n 调试模式：
  * `I18N_DEBUG_SHOW_KEY`
  * `I18N_DEBUG_SHOW_FALLBACK`
* `gui.cpp` 中用户可见文本的直接 A 版窗口文本读写已基本清空
* 一批高风险 ANSI / ACP 路径已完成首轮收口
* 已建立验证清单、路径分类说明与兼容边界审计文档

---

## 8.2 当前阶段明确暂缓的事项

以下内容当前**有意识地暂缓**，不是遗漏：

* 不重写 i18n 架构
* 不替换 JSON 方案
* 不做大规模 UI 框架重构
* 不一次性消灭所有 `CP_ACP` / ANSI 兼容路径
* 不直接重构历史脚本/配置解析模型

原因：

* 当前阶段以低风险、小步、可验证为优先
* 剩余问题中已有一部分属于第三方接口、注册表读取、历史脚本解析等兼容边界
* 继续硬改会明显提高回归风险

---

## 8.3 当前剩余问题应如何理解

当前仍存在的兼容问题，不应再简单理解为“工程仍然很乱”，而应理解为：

> **主链路已现代化，剩余问题已被压缩为少数明确、可观测、带兜底的兼容边界（legacy boundary）。**

这些边界当前主要集中在：

* `config.cpp`
* `output_asio.cpp`
* `synthesizer_vst.cpp`
* `asio/asiolist.cpp`
* `language.cpp` 的 legacy-only 接口

后续处理应继续遵循：

1. 先标记边界
2. 先补日志与失败保护
3. 再评估是否值得做结构性替换

---

## 8.4 下一阶段建议入口

后续如果继续推进，建议按以下顺序选择入口：

### 入口 A：验证与回归

优先执行：

* `doc/VALIDATION_CHECKLIST.md`
* 尤其是其中的“建议优先执行的最小验证集”

适用场景：

* 准备发布前自测
* 新维护者接手前确认状态
* 每轮改动后的回归检查

---

### 入口 B：UI 刷新体验继续补强

方向包括：

* 设置窗口当前页原地刷新
* TreeView / ListView / ComboBox 更细粒度刷新
* 更多 modeless 窗口纳入 `gui_refresh_all_texts()`

适用场景：

* 继续提升运行时语言切换体验

---

### 入口 C：兼容边界渐进治理

优先关注：

* `doc/ANSI_ACP_AUDIT.md`
* `doc/PATH_CLASSIFICATION.md`

适用场景：

* 继续减少历史兼容路径
* 但仍需保持低风险、渐进式原则

---

## 8.5 给后续维护者的一句话建议

> 不要急着重写；先按验证清单确认现状，再围绕日志、刷新、兼容边界这三条线做小步补强。

---

# 九、阶段完成说明（本轮封版总结）

## 9.1 本轮可以视为已完成的事项

截至当前状态，本轮工作已经完成以下内容：

### 工程能力补强

* 轻量日志系统已落地
* 主链路日志已接入
* Debug/Console 构建下可实时查看日志
* JSON i18n 主链路的可观测性已建立
* fallback / missing key / 非法 UTF-8 已可通过日志观察

### 调试与开发支持

* 已支持启动参数强制语言
* 已支持环境变量强制语言
* 已实现 i18n 调试模式：
  * `I18N_DEBUG_SHOW_KEY`
  * `I18N_DEBUG_SHOW_FALLBACK`

### GUI 与运行时体验

* 已支持运行时语言切换
* 已建立统一刷新入口 `gui_refresh_all_texts()`
* 设置窗口与 key setting 窗口在切语言时已有可控行为

### 路径与兼容边界治理

* `freepiano.cfg` 已明确归类为运行时配置文件，并默认位于 exe 同目录
* 文件对话框路径链路已集中封装
* 菜单文本不再回退到 `AppendMenuA`
* `gui.cpp` 中用户可见文本的直接 A 版窗口文本读写已基本清空
* 剩余 ANSI / ACP 问题已被压缩为少数兼容边界（legacy boundary）

### 文档与交付能力

* 已建立实现进展文档
* 已建立 ANSI / ACP 审计文档
* 已建立验证与回归检查清单
* 已建立路径分类说明
* 已建立交付式目录导航

### 编译质量

* `freepiano` 主工程当前构建日志中的 warning 已清理到 0（不含 `3rd/`）

---

## 9.2 当前明确未处理的事项

以下内容当前**明确未处理或仅做了边界收口**：

* 第三方库（`3rd/`）中的 warning
* 全量消除所有 `CP_ACP` 路径
* UI 复杂窗口的原地刷新重建
* 历史脚本/配置解析模型重构
* i18n 架构层级的大调整

---

## 9.3 为什么这些事项当前不处理

原因不是忽略，而是当前阶段有明确取舍：

* 第三方库 warning 不属于本轮核心收益区
* 剩余很多问题已经进入第三方接口或历史兼容边界
* 继续深挖将显著增加回归风险
* 当前阶段的更优策略是：
  * 先把主工程收干净
  * 先把关键用户路径收稳定
  * 先把剩余问题文档化、边界化、可观测化

这也是为什么本轮在“主工程无 warning、主路径稳定、文档完整”之后应视为一个合理收口点。

---

## 9.4 当前阶段的完成判定

如果以后需要判断“本轮是否已完成”，建议使用以下标准：

### 已满足

* 主工程可正常构建
* 主工程 warning 已清理到可接受范围（当前为 0，不含 `3rd/`）
* 关键功能可运行
* 日志系统正常
* 运行时语言切换正常
* 配置路径规则已修正
* 文档体系可支撑后续接手与维护

在当前状态下，上述条件已经满足，因此：

> **本轮工作可以视为阶段性完成。**

---

## 9.5 下一阶段如果继续推进，建议从哪里开始

后续继续推进时，建议从以下三个入口中择一，而不是重新发散：

### 入口 A：验证与回归

* 按 `doc/VALIDATION_CHECKLIST.md` 执行最小验证集
* 根据实际验证结果再决定修复优先级

### 入口 B：GUI 体验增强

* 继续完善设置页与复杂窗口的原地刷新能力

### 入口 C：兼容边界渐进治理

* 继续按 `doc/ANSI_ACP_AUDIT.md` 缩小兼容边界
* 但仍保持低风险、小步原则

---

## 9.6 封版一句话总结

> 本轮改造已经从“功能实现”走到了“工程收口”：主链路稳定、日志完善、文档齐全、兼容边界清晰、主工程编译质量可接受，适合作为当前阶段的封版点。
