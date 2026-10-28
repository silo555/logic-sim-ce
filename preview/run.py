#!/usr/bin/env python3
"""Run the actual C logic, UI, and pixel renderer in a local browser preview.
Requires Python 3 and a C compiler (macOS: Xcode command line tools).
"""
import ctypes
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import zlib
from http.server import BaseHTTPRequestHandler, HTTPServer

ROOT = Path(__file__).resolve().parents[1]
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8764
BUILD = tempfile.TemporaryDirectory(prefix="logic-ce-preview-")
library = Path(BUILD.name) / ("preview.dylib" if sys.platform == "darwin" else "preview.so")
subprocess.run([os.environ.get("CC", "cc"), "-std=c11", "-O2", "-shared", "-fPIC", "-I"+str(ROOT/"src"),
                *[str(ROOT/"src"/x) for x in ("logic.c", "app.c", "render.c", "save.c")],
                str(ROOT/"preview/bridge.c"), "-o", str(library)], check=True)
lib = ctypes.CDLL(str(library))
for name in ("preview_pixels", "preview_colors", "preview_save"):
    getattr(lib, name).restype = ctypes.POINTER(ctypes.c_uint8)
lib.preview_load.argtypes = [ctypes.POINTER(ctypes.c_uint8),ctypes.c_int]
SAVE_SIZE = lib.preview_save_size()
lib.preview_init()
SAVE = ROOT/"preview/circuit.raw"
if SAVE.exists():
    saved = SAVE.read_bytes()
    if len(saved) in (108,SAVE_SIZE):
        lib.preview_load((ctypes.c_uint8*len(saved)).from_buffer_copy(saved),len(saved))

def chunk(tag, data):
    return struct.pack(">I", len(data))+tag+data+struct.pack(">I", zlib.crc32(tag+data))

def png():
    pixels = ctypes.string_at(lib.preview_pixels(), 320*240)
    palette = ctypes.string_at(lib.preview_colors(), 36)
    scanlines = b"".join(b"\0"+pixels[y*320:(y+1)*320] for y in range(240))
    return (b"\x89PNG\r\n\x1a\n"+chunk(b"IHDR", struct.pack(">IIBBBBB",320,240,8,3,0,0,0))
            +chunk(b"PLTE",palette)+chunk(b"IDAT",zlib.compress(scanlines))+chunk(b"IEND",b""))

class Handler(BaseHTTPRequestHandler):
    def send(self, content, mime="application/json", status=200):
        self.send_response(status)
        self.send_header("Content-Type", mime)
        self.send_header("Content-Length", str(len(content)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(content)

    def do_GET(self):
        if self.path == "/":
            self.send((ROOT/"preview/index.html").read_bytes(), "text/html; charset=utf-8")
        elif self.path == "/icon.png":
            self.send((ROOT/"icon.png").read_bytes(), "image/png")
        elif self.path.startswith("/screen.png"):
            self.send(png(), "image/png")
        elif self.path == "/state":
            self.send(json.dumps({"cursor": lib.preview_cursor()+1, "screen":lib.preview_screen(), "cols":lib.preview_cols(), "rows":lib.preview_rows(),
                                  "values":[lib.preview_value(i) for i in range(lib.preview_count())],
                                  "types":[lib.preview_type(i) for i in range(lib.preview_count())]}).encode())
        else:
            self.send(b"Not found", "text/plain", 404)

    def do_POST(self):
        # Only accept same-origin requests from this local preview.
        if self.headers.get("Origin") not in (None, f"http://127.0.0.1:{PORT}", f"http://localhost:{PORT}"):
            self.send(b"Forbidden", "text/plain", 403);return
        if self.path != "/key" or int(self.headers.get("Content-Length",0)) > 80:
            self.send(b"Bad request", "text/plain", 400);return
        try:
            key = json.loads(self.rfile.read(int(self.headers.get("Content-Length",0))))["key"]
            if type(key) is not int or not 1 <= key <= 13:
                raise ValueError("invalid key")
        except (ValueError, KeyError, TypeError):
            self.send(b"Bad request", "text/plain", 400);return
        effect = lib.preview_key(key)
        if effect in (1,2):
            temporary = SAVE.with_suffix(".tmp")
            temporary.write_bytes(ctypes.string_at(lib.preview_save(),SAVE_SIZE))
            temporary.replace(SAVE)
        self.send(json.dumps({"effect":effect}).encode())

    def log_message(self, *args):
        pass

server = HTTPServer(("127.0.0.1",PORT), Handler)
print(f"logic sim CE preview: http://127.0.0.1:{PORT}", flush=True)
print("Same C engine, controls and 320x240 renderer. Desktop OS/storage backend; not CE emulation.", flush=True)
try:
    server.serve_forever()
except KeyboardInterrupt:
    pass
finally:
    server.server_close()
    BUILD.cleanup()
