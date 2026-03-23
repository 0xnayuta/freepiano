#include "pch.h"
#include "language.h"
#include "config.h"
#include "json_loader.h"

#include <vector>
#include <map>

static int lang_current;
static const wchar_t* string_names_w[FP_IDS_COUNT];
static const wchar_t* string_table_w[FP_LANG_COUNT][FP_IDS_COUNT];
static std::vector<wchar_t*> localized_wstrings;

// JSON string tables (when USE_JSON_TRANSLATIONS is enabled)
static std::map<int, JsonLoader::StringMap> json_string_tables;
static bool json_loaded = false;

// Convert UTF-8 to wide string, memory is managed internally
static wchar_t* utf8_to_wide_owned(const char *text) {
  if (text == NULL)
    return NULL;

  int wide_length = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
  if (wide_length <= 0)
    return NULL;

  wchar_t *wide_text = new wchar_t[wide_length];
  MultiByteToWideChar(CP_UTF8, 0, text, -1, wide_text, wide_length);
  localized_wstrings.push_back(wide_text);
  return wide_text;
}

// Convert UTF-8 to wide string with explicit length (for embedded nulls)
static wchar_t* utf8_to_wide_owned_len(const char *text, size_t len) {
  if (text == NULL || len == 0)
    return NULL;

  int wide_length = MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), NULL, 0);
  if (wide_length <= 0)
    return NULL;

  wchar_t *wide_text = new wchar_t[wide_length + 1];
  MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), wide_text, wide_length);
  wide_text[wide_length] = L'\0';  // Add null terminator at the end
  localized_wstrings.push_back(wide_text);
  return wide_text;
}

// Convert UTF-8 filter string with embedded nulls to wide string
static const wchar_t* utf8_filter_to_wide(const char *text) {
  if (text == NULL)
    return L"";

  // Find total length including embedded nulls (double-null terminated)
  int total_len = 0;
  const char *p = text;
  while (*p || *(p + 1)) {
    total_len++;
    p++;
  }
  total_len += 2;

  int wide_length = MultiByteToWideChar(CP_UTF8, 0, text, total_len, NULL, 0);
  if (wide_length <= 0)
    return L"";

  wchar_t *wide_text = new wchar_t[wide_length];
  MultiByteToWideChar(CP_UTF8, 0, text, total_len, wide_text, wide_length);

  localized_wstrings.push_back(wide_text);
  return wide_text;
}

// Get string name from ID
static const char* get_string_name(uint uid) {
  static const struct { uint id; const char* name; } id_to_name[] = {
#define STR_ENGLISH(id, str) { id, #id },
#define STR_SCHINESE(id, str)
#include "language_strdef.h"
#undef STR_ENGLISH
#undef STR_SCHINESE
  };

  for (const auto& entry : id_to_name) {
    if (entry.id == uid) {
      return entry.name;
    }
  }
  return "";
}

// Load JSON language file
static bool load_json_language(int lang_id) {
  const char* code = lang_get_code(lang_id);
  if (code == nullptr || strcmp(code, "auto") == 0) {
    return false;
  }

  // Try multiple paths
  const char* paths[] = {
    "locales/%s.json",
    "res/locales/%s.json",
    "../locales/%s.json",
    "../res/locales/%s.json",
  };

  for (const char* path_template : paths) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), path_template, code);

    JsonLoader::StringMap strings;
    if (JsonLoader::parse_file(path, strings)) {
      json_string_tables[lang_id] = strings;
      fprintf(stderr, "Loaded JSON locale: %s (%zu strings)\n", path, strings.size());
      return true;
    }
  }

  return false;
}

void lang_init() {
  // Initialize string names (needed for dialog localization)
  for (int id = 0; id < FP_IDS_COUNT; id++) {
    string_names_w[id] = L"";
    for (int lang = 0; lang < FP_LANG_COUNT; lang++) {
      string_table_w[lang][id] = L"";
    }
  }

  // Initialize string names from macro definitions
#define STR_ENGLISH(id, str) string_names_w[id] = utf8_to_wide_owned(#id);
#define STR_SCHINESE(id, str)
#include "language_strdef.h"
#undef STR_ENGLISH
#undef STR_SCHINESE

#if USE_JSON_TRANSLATIONS
  // Load JSON language files
  json_loaded = true;
  bool en_loaded = load_json_language(FP_LANG_ENGLISH);
  bool zh_loaded = load_json_language(FP_LANG_SCHINESE);

  if (!en_loaded) {
    fprintf(stderr, "Warning: Failed to load en.json, using fallback\n");
    json_loaded = false;
  }
#endif

  // Always load hardcoded strings as fallback
#define STR_ENGLISH(id, str) string_table_w[FP_LANG_ENGLISH][id] = utf8_to_wide_owned(str);
#define STR_SCHINESE(id, str) string_table_w[FP_LANG_SCHINESE][id] = utf8_to_wide_owned(str);
#include "language_strdef.h"
#undef STR_ENGLISH
#undef STR_SCHINESE

  // Auto choose language by default
  lang_set_current(FP_LANG_AUTO);
}

// Get default language from system or config
int lang_get_default() {
  int configured_language = config_get_ui_language();
  if (configured_language != FP_LANG_AUTO)
    return configured_language;

  LANGID id = GetUserDefaultUILanguage();
  switch (PRIMARYLANGID(id)) {
  case LANG_ENGLISH:
    return FP_LANG_ENGLISH;

  case LANG_CHINESE:
    return FP_LANG_SCHINESE;
  }
  return FP_LANG_ENGLISH;
}

// Select current language
void lang_set_current(int languageid) {
  if (languageid > FP_LANG_AUTO && languageid < FP_LANG_COUNT) {
    lang_current = languageid;
  }
  else {
    lang_current = lang_get_default();
  }
}

// Get current language
int lang_get_current() {
  return lang_current;
}

const char* lang_get_code(int languageid) {
  switch (languageid) {
  case FP_LANG_ENGLISH:
    return "en";
  case FP_LANG_SCHINESE:
    return "zh-CN";
  default:
    return "auto";
  }
}

int lang_from_code(const char* code) {
  if (code == NULL || code[0] == 0 || _stricmp(code, "auto") == 0)
    return FP_LANG_AUTO;

  if (_stricmp(code, "en") == 0 || _stricmp(code, "en-US") == 0)
    return FP_LANG_ENGLISH;

  if (_stricmp(code, "zh-CN") == 0 || _stricmp(code, "zh") == 0 || _stricmp(code, "zh-Hans") == 0)
    return FP_LANG_SCHINESE;

  return FP_LANG_AUTO;
}

// Load string from JSON or fallback to hardcoded
const wchar_t* lang_load_string_w(uint uid) {
  if (uid < 1 || uid >= FP_IDS_COUNT)
    return L"";

#if USE_JSON_TRANSLATIONS
  if (json_loaded) {
    auto it = json_string_tables.find(lang_current);
    if (it != json_string_tables.end()) {
      const char* name = get_string_name(uid);
      auto str_it = it->second.find(name);
      if (str_it != it->second.end()) {
        // Convert UTF-8 to wide and cache
        const wchar_t* cached = utf8_to_wide_owned(str_it->second.c_str());
        return cached ? cached : L"";
      }
    }

    // Fallback to English if current language doesn't have the string
    if (lang_current != FP_LANG_ENGLISH) {
      auto en_it = json_string_tables.find(FP_LANG_ENGLISH);
      if (en_it != json_string_tables.end()) {
        const char* name = get_string_name(uid);
        auto str_it = en_it->second.find(name);
        if (str_it != en_it->second.end()) {
          const wchar_t* cached = utf8_to_wide_owned(str_it->second.c_str());
          return cached ? cached : L"";
        }
      }
    }
  }
#endif

  // Fallback to hardcoded strings
  return string_table_w[lang_current][uid];
}

// Load string as narrow (converts from wide)
const char* lang_load_string(uint uid) {
  static char buffer[4096];
  const wchar_t* wstr = lang_load_string_w(uid);
  WideCharToMultiByte(CP_ACP, 0, wstr, -1, buffer, sizeof(buffer), NULL, NULL);
  return buffer;
}

// Format wide string
int lang_format_string_w(wchar_t *buff, size_t size, uint strid, ...) {
  const wchar_t *format = lang_load_string_w(strid);
  va_list args;
  va_start(args, strid);
  int result = _vsnwprintf(buff, size, format, args);
  va_end(args);
  if (size > 0)
    buff[size - 1] = 0;
  return result;
}

// Format narrow string
int lang_format_string(char *buff, size_t size, uint strid, ...) {
  const char *format = lang_load_string(strid);
  va_list args;
  va_start(args, strid);
  int result = _vsnprintf(buff, size, format, args);
  va_end(args);
  if (size > 0)
    buff[size - 1] = 0;
  return result;
}

// Load filter string for file dialogs
const wchar_t* lang_load_filter_w(uint uid) {
#if USE_JSON_TRANSLATIONS
  if (json_loaded) {
    auto it = json_string_tables.find(lang_current);
    if (it != json_string_tables.end()) {
      const char* name = get_string_name(uid);
      auto str_it = it->second.find(name);
      if (str_it != it->second.end()) {
        // Use data() and size() to preserve embedded null characters
        const std::string& str = str_it->second;
        return utf8_to_wide_owned_len(str.data(), str.size());
      }
    }

    // Fallback to English if current language doesn't have the filter
    if (lang_current != FP_LANG_ENGLISH) {
      auto en_it = json_string_tables.find(FP_LANG_ENGLISH);
      if (en_it != json_string_tables.end()) {
        const char* name = get_string_name(uid);
        auto str_it = en_it->second.find(name);
        if (str_it != en_it->second.end()) {
          const std::string& str = str_it->second;
          return utf8_to_wide_owned_len(str.data(), str.size());
        }
      }
    }
  }
#endif

  // Fallback to hardcoded filter strings
  static const char* filter_strings[] = {
#define STR_ENGLISH(id, str) str,
#define STR_SCHINESE(id, str)
#include "language_strdef.h"
#undef STR_ENGLISH
#undef STR_SCHINESE
  };

  if (uid < FP_IDS_COUNT) {
    // Find the filter string by ID
    static bool init = false;
    static const char* filter_map[FP_IDS_COUNT];

    if (!init) {
      memset(filter_map, 0, sizeof(filter_map));
#define STR_ENGLISH(id, str) if (strstr(#id, "FILTER") || strstr(#id, "OPEN_FILTER") || strstr(#id, "SAVE_FILTER")) filter_map[id] = str;
#define STR_SCHINESE(id, str)
#include "language_strdef.h"
#undef STR_ENGLISH
#undef STR_SCHINESE
      init = true;
    }

    if (filter_map[uid]) {
      return utf8_filter_to_wide(filter_map[uid]);
    }
  }

  return L"";
}

// Load string array (legacy)
const char* * lang_load_string_array(uint uid) {
  static char temp[1024];
  static char *arr[256];
  const uint temp_size = sizeof(temp) / sizeof(temp[0]);
  uint len = LoadStringA(GetModuleHandle(NULL), uid, temp, temp_size - 1);
  temp[len < temp_size ? len : 0] = 0;

  arr[0] = temp;
  uint arrlen = 1;

  for (uint i = 0; i < len; i++) {
    if (temp[i] == ',') {
      temp[i] = '\0';
      if (arrlen < 256) {
        arr[arrlen] = temp + i + 1;
        arrlen++;
      }
    }
  }
  arr[arrlen] = 0;
  return (const char * *)arr;
}

// Text resource handling
static struct lang_text_t {
  char *data;
  char *end;
  HGLOBAL hrc;
} lang_text = { NULL, NULL, NULL };

static WORD system_language(int lang_id) {
  switch (lang_id) {
  case FP_LANG_ENGLISH:  return MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
  case FP_LANG_SCHINESE: return MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
  }
  return MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
}

int lang_text_open(uint textid) {
  lang_text_close();

  HINSTANCE module = GetModuleHandle(NULL);
  HRSRC hrsrc = FindResourceExA(module, "TEXT", MAKEINTRESOURCEA(textid), system_language(lang_current));

  if (hrsrc == NULL)
    hrsrc = FindResourceExA(module, "TEXT", MAKEINTRESOURCEA(textid), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

  if (hrsrc) {
    HGLOBAL hrc = LoadResource(0, hrsrc);
    if (hrc) {
      lang_text.hrc = LoadResource(0, hrsrc);
      lang_text.data = (char *)LockResource(hrc);
      lang_text.end = lang_text.data + SizeofResource(NULL, hrsrc);
      return lang_text.data - lang_text.end;
    }
  }

  return 0;
}

int lang_text_readline(char *line, size_t size) {
  char *line_end = line;

  while (lang_text.data < lang_text.end) {
    if (*lang_text.data == '\n') {
      if (line_end > line) {
        *line_end = 0;
        lang_text.data++;
        return line_end - line;
      }
    } else {
      if (line_end < line + size - 1)
        *line_end++ = *lang_text.data;
    }
    lang_text.data++;
  }

  if (line_end > line) {
    *line_end = 0;
    return line_end - line;
  }

  return 0;
}

void lang_text_close() {
  if (lang_text.hrc)
    FreeResource(lang_text.hrc);

  lang_text.data = NULL;
  lang_text.end = NULL;
  lang_text.hrc = NULL;
}

// Error handling
static char error_message[1024] = "Unknown error.";
static wchar_t error_message_w[1024] = L"Unknown error.";

static void sync_last_error_wide_from_narrow() {
  MultiByteToWideChar(CP_ACP, 0, error_message, -1, error_message_w, ARRAYSIZE(error_message_w));
}

void lang_set_last_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf_s(error_message, sizeof(error_message), _TRUNCATE, format, args);
  va_end(args);
  sync_last_error_wide_from_narrow();
  fprintf(stderr, "last_error: %s\n", error_message);
}

void lang_set_last_error(uint id, ...) {
  const char *format = lang_load_string(id);
  va_list args;
  va_start(args, id);
  vsnprintf_s(error_message, sizeof(error_message), _TRUNCATE, format, args);
  va_end(args);
  sync_last_error_wide_from_narrow();
  fprintf(stderr, "last_error: %s\n", error_message);
}

const char * lang_get_last_error() {
  return error_message;
}

const wchar_t * lang_get_last_error_w() {
  return error_message_w;
}

// Dialog localization
static void set_window_text_localized(HWND hwnd, const wchar_t *text) {
  SetWindowTextW(hwnd, text);
}

static BOOL CALLBACK localize_hwnd(HWND hwnd, LPARAM lParam) {
  wchar_t className[256];
  wchar_t text[64];
  GetClassNameW(hwnd, className, ARRAYSIZE(className));
  GetWindowTextW(hwnd, text, ARRAYSIZE(text));

  for (int i = 1; i < FP_IDS_COUNT; i++) {
    if (wcscmp(text, string_names_w[i]) == 0) {
      set_window_text_localized(hwnd, lang_load_string_w(i));
      return TRUE;
    }
  }

  return TRUE;
}

void lang_localize_dialog(HWND hwnd) {
  localize_hwnd(hwnd, NULL);
  EnumChildWindows(hwnd, &localize_hwnd, NULL);
}
