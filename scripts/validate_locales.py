#!/usr/bin/env python3
"""
Validate FreePiano locale JSON files.

Checks that all locale files have the same keys as the English reference.

Usage:
    python validate_locales.py [--locales-dir DIR]

Exit codes:
    0 - All locales are valid
    1 - Missing or extra keys found
"""

import json
import os
import sys
import argparse


def load_json(path):
    """Load a JSON file with UTF-8 encoding."""
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def get_string_keys(data):
    """Extract string keys from a locale JSON data structure."""
    if 'strings' not in data:
        return set()
    return set(data['strings'].keys())


def validate_single_locale(filepath, reference_keys):
    """Validate a single locale file against reference keys."""
    try:
        data = load_json(filepath)
    except json.JSONDecodeError as e:
        return False, [f"JSON parse error: {e}"]
    except FileNotFoundError:
        return False, [f"File not found: {filepath}"]

    lang_keys = get_string_keys(data)

    missing = reference_keys - lang_keys
    extra = lang_keys - reference_keys

    errors = []
    if missing:
        errors.append(f"Missing keys: {sorted(missing)}")
    if extra:
        errors.append(f"Unknown keys: {sorted(extra)}")

    return len(errors) == 0, errors


def validate_all_locales(locales_dir):
    """Validate all locale files in the directory."""
    # Find the English reference file
    en_path = os.path.join(locales_dir, 'en.json')
    if not os.path.exists(en_path):
        print(f"Error: Reference file not found: {en_path}", file=sys.stderr)
        return False

    # Load reference keys
    try:
        en_data = load_json(en_path)
        reference_keys = get_string_keys(en_data)
        print(f"Reference (en.json): {len(reference_keys)} strings")
    except Exception as e:
        print(f"Error loading reference: {e}", file=sys.stderr)
        return False

    # Validate all locales
    all_valid = True
    for filename in sorted(os.listdir(locales_dir)):
        if not filename.endswith('.json'):
            continue

        filepath = os.path.join(locales_dir, filename)

        if filename == 'en.json':
            print(f"  [OK] {filename} (reference)")
            continue

        valid, errors = validate_single_locale(filepath, reference_keys)

        if valid:
            data = load_json(filepath)
            lang_name = data.get('_meta', {}).get('name', '?')
            n_strings = len(data.get('strings', {}))
            print(f"  [OK] {filename} ({lang_name}): {n_strings} strings")
        else:
            all_valid = False
            print(f"  [ERROR] {filename}:", file=sys.stderr)
            for error in errors:
                print(f"    {error}", file=sys.stderr)

    return all_valid


def main():
    parser = argparse.ArgumentParser(description='Validate FreePiano locale files')
    parser.add_argument('--locales-dir', '-d',
                        default='res/locales',
                        help='Directory containing locale JSON files')
    args = parser.parse_args()

    locales_dir = args.locales_dir
    if not os.path.isabs(locales_dir):
        # Try to find the directory relative to the script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(script_dir)
        locales_dir = os.path.join(project_root, locales_dir)

    print(f"Validating locales in: {locales_dir}")
    print()

    if validate_all_locales(locales_dir):
        print()
        print("All locale files are valid!")
        return 0
    else:
        print()
        print("Validation failed!", file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
