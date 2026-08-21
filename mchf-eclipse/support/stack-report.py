#!/usr/bin/env python3
"""
Parse GCC LTO stack-usage (.su) files and print top-10 deepest call-stacks per target.

Usage:
    python3 support/stack-report.py [directory]

If no directory is given, uses the current working directory.
"""

import os
import sys


def parse_su_file(filepath):
    """Parse a single .su file and return list of (func, stack_bytes, source)."""
    entries = []
    with open(filepath, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            # Expected format:
            #   path:line:column:function_name\tstack_size:static_or_global
            parts = line.split("\t", 1)
            if len(parts) != 2:
                continue
            left, right = parts
            # right = "48 static" (space-separated stack size and linkage)
            right_parts = right.strip().split()
            if len(right_parts) < 1:
                continue
            stack_str = right_parts[0]
            try:
                stack = int(stack_str)
            except ValueError:
                continue
            # Extract function name from left part: path:line:column:function_name
            left_parts = left.split(":")
            if len(left_parts) < 4:
                continue
            func = left_parts[3]
            entries.append((func, stack, left))
    return entries


def find_su_files(directory):
    """Find all .su files and group them by target prefix."""
    files = []
    for root, _, filenames in os.walk(directory):
        for name in filenames:
            if name.endswith(".su"):
                files.append(os.path.join(root, name))
    return files


def group_by_target(su_files):
    """Group .su files by target (fw-* / bl-* / other)."""
    groups = {}
    for path in su_files:
        name = os.path.basename(path)
        if name.startswith("fw-"):
            target = "firmware"
        elif name.startswith("bl-"):
            target = "bootloader"
        else:
            target = "other"
        groups.setdefault(target, []).append(path)
    return groups


def main():
    directory = sys.argv[1] if len(sys.argv) > 1 else "."
    su_files = find_su_files(directory)
    if not su_files:
        print(f"No .su files found in {directory}")
        sys.exit(1)

    groups = group_by_target(su_files)

    for target in ("firmware", "bootloader"):
        files = groups.get(target, [])
        if not files:
            continue
        entries = []
        for path in files:
            entries.extend(parse_su_file(path))
        if not entries:
            continue
        entries.sort(key=lambda e: e[1], reverse=True)
        top = entries[:10]

        print(f"Top 10 deepest call-stacks ({target}):")
        print(f"{'Stack':>8}  {'Function'}")
        print(f"{'-----':>8}  {'-------'}")
        for func, stack, source in top:
            print(f"{stack:>8}  {func}")
        print()


if __name__ == "__main__":
    main()
