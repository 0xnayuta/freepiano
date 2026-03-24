# FreePiano Changelog（2026-03）

适用范围：本轮 Unicode / i18n / 工程质量补强工作总结

---

## Added

* 新增轻量日志系统：
  * `src/fp_log.h`
  * `src/fp_log.cpp`
* 新增日志输出目标：
  * `OutputDebugStringW`
  * `logs/freepiano.log`
  * Debug/Console 构建下控制台实时日志输出
* 新增 i18n 调试模式：
  * `I18N_DEBUG_SHOW_KEY`
  * `I18N_DEBUG_SHOW_FALLBACK`
* 新增启动期语言覆盖能力：
  * `FREEPIANO_LANG`
  * `--lang=en`
  * `--lang=zh-CN`
* 新增统一 UI 刷新入口：
  * `gui_refresh_all_texts()`
* 新增路径与文本转换辅助封装：
  * 文件对话框路径转换封装
  * GUI 用户可见文本统一读写封装
  * ASIO / VST / config 兼容边界辅助转换函数
* 新增文档：
  * `doc/IMPLEMENTATION_PROGRESS.md`
  * `doc/ANSI_ACP_AUDIT.md`
  * `doc/VALIDATION_CHECKLIST.md`
  * `doc/PATH_CLASSIFICATION.md`
  * `doc/HANDOFF_GUIDE.md`

---

## Changed

* `language.cpp` 改为更强的防御性实现：
  * UTF-8 严格校验
  * missing key / fallback / invalid UTF-8 日志
  * safer fallback 逻辑
* `json_loader.cpp` 增加 JSON 打开/读取/解析失败日志
* `main.cpp` 增加启动日志与语言覆盖逻辑
* `gui.cpp` 增加统一语言刷新入口，并把语言切换流程收口到 `gui_refresh_all_texts()`
* `config.cpp` 调整 `freepiano.cfg` 默认路径规则：
  * 改为 exe 同目录
* `gui.cpp` 中用户可见文本的直接 A 版窗口文本读写已基本清空
* `output_asio.cpp` / `synthesizer_vst.cpp` / `asio/asiolist.cpp` 的兼容边界已集中、加注释、加日志
* `language.h` / `language.cpp` 明确标记 `lang_load_string_array()` 为 legacy-only 兼容接口

---

## Fixed

* 修复运行时语言切换后主菜单刷新缺少统一入口的问题
* 修复 Debug 下 `freepiano.cfg` 默认落到历史 `data` 路径导致加载/保存异常的问题
* 修复 Debug 控制台打开但无实时日志输出的问题
* 修复一批 `stderr` / `stdout` / `printf` 历史输出未统一进入日志系统的问题
* 修复一批用户可见文本链路仍直接使用 `SetWindowTextA` / `GetWindowTextA` 等 API 的问题
* 修复 song info / keymap / key setting / 设置页中若干文本读写链路的兼容问题
* 修复主工程中一批 `size_t -> int/DWORD` 的 warning
* 修复 `song.cpp` 中 `read_idp_string()` 读取长度错误的潜在问题

---

## Notes

* 当前 `freepiano` 主工程构建 warning 已清理到 0（不含 `3rd/`）
* 第三方库 warning 仍保留，当前未纳入本轮修复范围
* 当前阶段不建议：
  * 重写 i18n 架构
  * 一次性清除所有 `ANSI / ACP` 路径
  * 大规模重构 GUI 或历史脚本/配置解析模型
* 当前更适合的后续入口：
  * 按 `doc/VALIDATION_CHECKLIST.md` 做验证回归
  * 继续完善 GUI 原地刷新能力
  * 按 `doc/ANSI_ACP_AUDIT.md` 渐进治理剩余兼容边界

---

## One-line Summary

> 本轮工作已将 FreePiano 从“完成 Unicode / i18n 主改造”推进到“具备日志、调试、验证、路径分类与兼容边界治理能力”的阶段，适合作为当前阶段的封版结果。
