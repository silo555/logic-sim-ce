#!/usr/bin/env python3
"""Pixel-exact 16x16 Cesium icon; no image-library dependencies."""
from pathlib import Path
import struct
import zlib
ROOT=Path(__file__).resolve().parents[1]
PALETTE={'.':(12,20,32),'b':(28,48,68),'s':(48,76,96),'g':(64,208,128),
         'h':(144,248,184),'w':(224,248,248),'a':(248,192,64),'y':(255,240,160)}
pixels=[['.']*16 for _ in range(16)]
def rect(x,y,w,h,c):
    for yy in range(y,y+h):
        for xx in range(x,x+w): pixels[yy][xx]=c
# Rounded tile rim, small microchip pins, and a luminous AND-gate symbol.
rect(2,0,12,1,'b');rect(1,1,14,1,'s');rect(0,2,1,12,'b');rect(15,2,1,12,'b')
rect(1,14,14,1,'b');rect(2,15,12,1,'b')
for y in (5,8,11):rect(2,y,2,1,'g');rect(12,y,2,1,'s')
for x in (5,8,11):rect(x,2,1,2,'s');rect(x,12,1,2,'g')
rect(4,4,8,8,'g');rect(5,5,6,6,'b');rect(4,4,8,1,'h')
# White D shape reads as a logic gate even at the real 16-pixel size.
rect(6,6,1,4,'w');rect(7,6,2,1,'w');rect(9,7,1,2,'w');rect(7,9,2,1,'w')
rect(3,6,3,1,'h');rect(3,9,3,1,'h');rect(10,8,4,1,'g')
rect(13,7,2,3,'a');rect(13,7,1,1,'y')
# A dark lower edge makes the chip pop without antialiasing.
rect(5,11,6,1,'s')
def chunk(t,d):return struct.pack('>I',len(d))+t+d+struct.pack('>I',zlib.crc32(t+d))
def png(path,scale):
    w=16*scale;raw=bytearray()
    for y in range(w):
        raw.append(0)
        for x in range(w):raw.extend(PALETTE[pixels[y//scale][x//scale]])
    path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,w,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b''))
png(ROOT/'icon.png',1)
if __name__=='__main__':
    import sys
    if len(sys.argv)>1:png(Path(sys.argv[1]),12)
