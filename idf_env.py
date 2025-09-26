#!/usr/bin/env python3

import os
import sys


def get_default_idf_path():
    user_profile_dir = os.path.expanduser("~")
    esp_idf_dir = os.path.join(user_profile_dir, "esp")
    if not os.path.isdir(esp_idf_dir):
        return None

    # Find max version in the directory (vX.Y.Z)
    version_dirs = [
        d for d in os.listdir(esp_idf_dir)
        if os.path.isdir(os.path.join(esp_idf_dir, d)) and d.startswith("v")
    ]
    if not version_dirs:
        return None

    max_version = max(version_dirs, key=lambda v: list(map(int, v[1:].split("."))))
    return os.path.join(esp_idf_dir, max_version, "esp-idf")


def main():
    idf_path = os.getenv("IDF_PATH", get_default_idf_path())
    if not idf_path:
        print("ESP-IDF path not found. Please set the IDF_PATH environment variable.")
        return 1

    args = ' '.join(f'{arg}' for arg in sys.argv[1:])
    if os.name == "posix":  # macOS or Linux
        if args:
            os.system(f'/bin/bash -c "source {idf_path}/export.sh && {args}"')
        else:
            os.system(
                f'/usr/bin/env bash --noprofile --norc -ic '
                f'"source \\"{idf_path}/export.sh\\"; PS1=\\"(idf) $ \\" exec bash --noprofile --norc -i"'
            )
    elif os.name == "nt":  # Windows
        if args:
            os.system(f'"{idf_path}\\export.bat" && {args}')
        else:
            os.system(f'cmd /k "{idf_path}\\export.bat"')
    else:
        print("Unsupported OS. This script supports only Windows, macOS, and Linux.")
        return 1

    return 0


if __name__ == "__main__":
    ret = main()
    exit(ret)
