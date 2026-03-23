#!/usr/bin/env python3
"""
Generate JSON locale files from language_strdef.h
"""

import re
import json
import os

def decode_unicode_escapes(text):
    """Decode \\uXXXX escape sequences to actual Unicode characters."""
    def replace_escape(match):
        return chr(int(match.group(1), 16))

    return re.sub(r'\\u([0-9A-Fa-f]{4})', replace_escape, text)

def convert_c_escapes(text):
    """Convert C escape sequences to actual characters.

    In the source file:
    - \\0 becomes null character (for file filters)
    - \\uXXXX becomes the actual Unicode character
    """
    # First decode \\uXXXX sequences
    result = decode_unicode_escapes(text)
    # Then convert C \0 escape to actual null character
    result = result.replace('\\0', '\0')
    return result

def parse_language_strdef(filepath):
    """Parse language_strdef.h and extract all strings."""
    with open(filepath, 'r', encoding='utf-8-sig') as f:
        content = f.read()

    english = {}
    chinese = {}

    # Process line by line
    for line in content.split('\n'):
        line = line.strip()
        if not line:
            continue

        # English strings: STR_ENGLISH  (ID, "value") or STR_ENGLISH(ID, u8"value")
        en_match = re.match(r'STR_ENGLISH\s*\(\s*(IDS_[A-Za-z_0-9]+)\s*,\s*(?:u8)?"(.*)"\s*\)', line)
        if en_match:
            val = en_match.group(2)
            # Convert C escape sequences
            val = convert_c_escapes(val)
            english[en_match.group(1)] = val
            continue

        # Chinese strings: STR_SCHINESE (ID, u8"value")
        zh_match = re.match(r'STR_SCHINESE\s*\(\s*(IDS_[A-Za-z_0-9]+)\s*,\s*(?:u8)?"(.*)"\s*\)', line)
        if zh_match:
            val = zh_match.group(2)
            # Convert C escape sequences
            val = convert_c_escapes(val)
            chinese[zh_match.group(1)] = val
            continue

    return english, chinese

def generate_json(english, chinese, output_dir):
    """Generate JSON locale files."""

    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Generate en.json
    en_json = {
        "_meta": {
            "language": "en",
            "name": "English",
            "version": "1.8"
        },
        "strings": english
    }

    with open(os.path.join(output_dir, 'en.json'), 'w', encoding='utf-8') as f:
        json.dump(en_json, f, ensure_ascii=False, indent=2)

    print(f"Generated en.json with {len(english)} strings")

    # Generate zh-CN.json
    # Use English as base, then override with Chinese translations
    zh_strings = dict(english)  # Start with English
    zh_strings.update(chinese)   # Override with Chinese

    zh_json = {
        "_meta": {
            "language": "zh-CN",
            "name": "简体中文",
            "version": "1.8"
        },
        "strings": zh_strings
    }

    with open(os.path.join(output_dir, 'zh-CN.json'), 'w', encoding='utf-8') as f:
        json.dump(zh_json, f, ensure_ascii=False, indent=2)

    print(f"Generated zh-CN.json with {len(zh_strings)} strings ({len(chinese)} translated)")

    # Report missing translations
    missing = set(english.keys()) - set(chinese.keys())
    if missing:
        print(f"\nNote: {len(missing)} strings use English fallback:")
        for m in sorted(missing)[:10]:
            print(f"  {m}: {english[m][:40]}...")
        if len(missing) > 10:
            print(f"  ... and {len(missing) - 10} more")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir) if script_dir else os.getcwd()

    strdef_path = os.path.join(project_root, 'src', 'language_strdef.h')
    output_dir = os.path.join(project_root, 'res', 'locales')

    print(f"Parsing: {strdef_path}")
    english, chinese = parse_language_strdef(strdef_path)

    print(f"\nFound {len(english)} English strings")
    print(f"Found {len(chinese)} Chinese translations")

    generate_json(english, chinese, output_dir)

    print("\nDone!")

if __name__ == '__main__':
    main()
