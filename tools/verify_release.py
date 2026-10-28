#!/usr/bin/env python3
"""Validate a built TI program and its embedded Cesium branding (stdlib only)."""
import hashlib
from pathlib import Path
import struct
import sys

EXPECTED_ICON_SHA256 = "f7f494db51cbc02f01eb3aeaad307c868dbf8cedda7c9c3f9120bc532b23622a"

def verify(path):
    data = path.read_bytes()
    if len(data) < 354:
        raise ValueError("file is too short")
    if data[:10] != b"**TI83F*\x1a\x0a":
        raise ValueError("invalid TI signature")
    u16 = lambda at: struct.unpack_from("<H", data, at)[0]
    if len(data) != 57 + u16(53) or u16(55) != 13:
        raise ValueError("invalid file lengths or metadata header")
    if data[59] != 6 or data[60:68] != b"LOGICCE\0":
        raise ValueError("wrong variable name or program type")
    if u16(57) != u16(70) or u16(72) != len(data) - 76:
        raise ValueError("variable payload lengths differ")
    if u16(len(data)-2) != sum(data[55:-2]) % 65536:
        raise ValueError("TI checksum mismatch")
    if data[74:78] != b"\xef\x7b\0\xc3" or data[81:84] != b"\1\x10\x10":
        raise ValueError("native header or 16x16 Cesium icon missing")
    if hashlib.sha256(data[84:340]).hexdigest() != EXPECTED_ICON_SHA256:
        raise ValueError("embedded sprite differs from the release icon")
    if data[340:353] != b"logic sim CE\0":
        raise ValueError("spaced Cesium description missing")
    print(f"PASS {path.name}: {len(data)} bytes; TI envelope, checksum, icon and name")
    print("SHA-256:",hashlib.sha256(data).hexdigest())

if __name__ == "__main__":
    try:
        verify(Path(sys.argv[1]) if len(sys.argv)>1 else Path("bin/LOGICCE.8xp"))
    except (OSError,ValueError,struct.error) as exc:
        sys.exit(str(exc))
