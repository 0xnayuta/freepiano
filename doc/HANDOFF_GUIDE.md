# FreePiano 交付式目录导航（2026-03）

更新时间：2026-03-24

---

# 一、这份文档是给谁看的

这份文档面向：

* 后续继续维护 FreePiano 的开发者
* 需要快速理解当前项目状态的接手人
* 需要定位“先看哪里、后看哪里”的协作者

目标：

> 用最短路径帮助接手人快速建立对当前项目的整体认知。

---

# 二、建议阅读顺序（文档）

如果你是第一次接手当前代码状态，建议按以下顺序阅读。

## 第 1 步：先看项目总体状态

文件：

* `doc/PROJECT_STATUS.md`

你会知道：

* 当前项目处于什么阶段
* 哪些内容已经完成
* 哪些内容仍保留为兼容边界
* 当前阶段的工作原则是什么

适合回答的问题：

* 现在还能不能继续大改架构？
* 这个项目目前最重要的目标是什么？
* 为什么文档里反复强调“小步、低风险、可验证”？

---

## 第 2 步：看本轮具体做了什么

文件：

* `doc/IMPLEMENTATION_PROGRESS.md`

你会知道：

* 本轮新增了哪些模块与能力
* 哪些文件被改过
* 当前阶段已经完成了哪些关键事项
* 哪些事项是有意识暂缓的
* 下一阶段从哪里继续最合理

适合回答的问题：

* 这轮工作到底改了什么？
* 为什么现在有日志、调试开关、语言覆盖、统一刷新入口？
* 还剩哪些问题没有动？

---

## 第 3 步：看兼容边界与风险点

文件：

* `doc/ANSI_ACP_AUDIT.md`

你会知道：

* 目前哪些 `ANSI / ACP` 路径仍然存在
* 哪些已经完成首轮收口
* 哪些点属于“兼容边界（legacy boundary）”
* 为什么当前不继续硬改这些边界

适合回答的问题：

* 为什么项目里还有 `CP_ACP`？
* 哪些地方已经安全，哪些地方还有历史兼容风险？
* 继续做 Unicode 化时该从哪里下手？

---

## 第 4 步：看验证与回归怎么做

文件：

* `doc/VALIDATION_CHECKLIST.md`

你会知道：

* 本轮改动应该如何验证
* 最小验证集是什么
* 回归测试优先测哪些点
* 当前阶段的验收标准是什么

适合回答的问题：

* 改完代码后先测什么？
* 如何快速确认语言切换、日志、路径链路没坏？
* 哪些点最容易回归？

---

## 第 5 步：看路径规则

文件：

* `doc/PATH_CLASSIFICATION.md`

你会知道：

* `freepiano.cfg` 为什么放在 exe 同目录
* 什么叫“运行时配置文件”
* 什么叫“历史资源 / 数据文件”
* 当前 `config_get_media_path()` 为什么不是所有相对路径都同规则

适合回答的问题：

* 为什么配置文件和 data 资源不是同一路径规则？
* Debug / Release 下相对路径为什么不同？
* 新增文件时该走配置路径还是资源路径？

---

# 三、建议阅读顺序（源码）

如果文档已经看完，接下来建议按以下顺序读源码。

## 3.1 启动主链路

文件：

* `src/main.cpp`

重点看：

* 程序启动流程
* `config_init()` / `config_preload_ui_language()`
* `lang_init()`
* 启动期语言覆盖：
  * `FREEPIANO_LANG`
  * `--lang=...`

如果你想理解：

* 为什么命令行语言覆盖比配置文件优先级高
* 为什么日志在启动初期就可用

请先看这里。

---

## 3.2 日志系统

文件：

* `src/fp_log.h`
* `src/fp_log.cpp`

重点看：

* `fp_log_info / warn / error`
* `OutputDebugStringW`
* 日志文件输出
* Debug/Console 构建下的控制台实时输出

如果你想继续扩展日志，先从这里入手。

---

## 3.3 语言系统主实现

文件：

* `src/language.h`
* `src/language.cpp`
* `src/json_loader.h`
* `src/json_loader.cpp`

重点看：

* JSON locale 加载
* fallback 逻辑
* missing key / invalid UTF-8 日志
* `I18N_DEBUG_SHOW_KEY`
* `I18N_DEBUG_SHOW_FALLBACK`
* `lang_load_filter_w()`
* legacy-only 接口 `lang_load_string_array()`

如果你后续要继续处理 i18n，这一组文件是核心入口。

---

## 3.4 GUI 刷新与语言切换

文件：

* `src/gui.h`
* `src/gui.cpp`

重点看：

* `gui_set_language()`
* `gui_refresh_all_texts()`
* 菜单重建
* 设置窗口 / key setting 窗口的刷新策略
* 文件对话框路径转换封装
* 用户可见文本的统一读写封装

如果你后续要继续提升“运行时切换语言体验”，这里是第一入口。

---

## 3.5 配置系统

文件：

* `src/config.h`
* `src/config.cpp`

重点看：

* `config_load()` / `config_save()`
* `config_get_media_path()`
* `freepiano.cfg` 的特殊路径规则
* 历史名表兼容边界

如果你后续要继续整理路径或配置行为，这里是核心入口。

---

## 3.6 第三方边界点

文件：

* `src/output_asio.cpp`
* `src/synthesizer_vst.cpp`
* `src/asio/asiolist.cpp`

重点看：

* ASIO 驱动/设备名的窄字符边界
* VST 插件名边界
* 注册表 CLSID 文本边界
* 已集中封装的转换辅助函数

如果你后续要继续治理 `ANSI / ACP` 边界，这几处是重点。

---

# 四、按目标查找文件（速查）

## 我想看“当前项目为什么这么设计”

先看：

1. `doc/PROJECT_STATUS.md`
2. `doc/IMPLEMENTATION_PROGRESS.md`

---

## 我想看“还有哪些历史兼容问题”

先看：

1. `doc/ANSI_ACP_AUDIT.md`
2. `src/config.cpp`
3. `src/output_asio.cpp`
4. `src/synthesizer_vst.cpp`
5. `src/asio/asiolist.cpp`

---

## 我想看“语言切换为什么能运行时生效”

先看：

1. `src/main.cpp`
2. `src/language.cpp`
3. `src/gui.cpp`

---

## 我想看“为什么配置文件现在在 exe 同目录”

先看：

1. `doc/PATH_CLASSIFICATION.md`
2. `src/config.cpp`

---

## 我想看“怎么验证本轮改动是否没坏”

先看：

1. `doc/VALIDATION_CHECKLIST.md`

---

# 五、当前阶段不建议做的事

如果你是后续维护者，当前阶段**不建议上来就做**：

* 重写 i18n 架构
* 替换 JSON 方案
* 大规模重构 GUI 框架
* 一次性消灭所有 `CP_ACP`
* 直接改历史脚本/配置解析模型

原因：

* 当前阶段已经把高风险问题压缩到了少数边界点
* 再往下做就会快速进入结构性演进，回归风险明显上升

建议原则：

> 先验证，再小步补强；先看文档，再动核心路径。

---

# 六、建议的后续推进方式

## 路线 A：继续做功能质量

适合目标：

* 提升运行时语言切换体验
* 继续完善 UI 原地刷新

建议入口：

* `src/gui.cpp`
* `doc/VALIDATION_CHECKLIST.md`

---

## 路线 B：继续做兼容边界治理

适合目标：

* 继续减少历史 `ANSI / ACP` 残留

建议入口：

* `doc/ANSI_ACP_AUDIT.md`
* `doc/PATH_CLASSIFICATION.md`
* `src/config.cpp`
* `src/output_asio.cpp`
* `src/synthesizer_vst.cpp`

---

## 路线 C：继续做验证与交付质量

适合目标：

* 发布前整理
* 接手前确认
* 建立稳定回归流程

建议入口：

* `doc/VALIDATION_CHECKLIST.md`
* `doc/IMPLEMENTATION_PROGRESS.md`

---

# 七、最后一句话

> 如果你是第一次接手当前代码：先看 `PROJECT_STATUS.md`，再看 `IMPLEMENTATION_PROGRESS.md`，然后按 `VALIDATION_CHECKLIST.md` 做最小验证；只有在确认现状稳定后，再进入 GUI 刷新或兼容边界治理。
