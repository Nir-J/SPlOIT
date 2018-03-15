## Team5

### Command Injection

* Parameters for echo are not filtered. Hence we can run arbitrary commands.

```
$ echo hello `date`
hello Wed Mar 14 20:18:11 EDT 2018

$ echo hello $(date)
hello Wed Mar 14 20:19:50 EDT 2018

$ echo hello | date
Wed Mar 14 20:20:22 EDT 2018

$ echo hello & date
Wed Mar 14 20:20:46 EDT 2018

$ echo hello || date
Wed Mar 14 20:22:13 EDT 2018

```

### Directory traversal

* Absolute paths are not handled.
```
$ cd /

$ ls
total 96
drwxr-xr-x   2 root root  12288 Mar 13 05:09 bin
drwxr-xr-x   4 root root   4096 Mar  6 05:09 boot
drwxr-xr-x  21 root root   5380 Oct 25 16:16 dev
drwxr-xr-x 136 root root  12288 Mar 13 05:09 etc
drwxr-xr-x   2 root root   4096 Apr 12  2016 home
...
```
* Server thread crashed when user sends cd command before logging in.

```C
else if(!strcmp(command->cmd, "cd")){
  if (current_user==NULL){
    if(current_user->isLoggedIn==false){ // NULL->isLoggedIn? Crash!
    
    }
  }
```

### Buffer Oveflow

* File name has a limited char array size of 50, but this bound is not checked.

```
$ get AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAA

$ 

$ 
mc08 75 $ 
```

### Design Bugs

* Passing pointer to local variables
```C
struct arg arguments;
arguments.filepath = filepath;
arguments.port = dataport;
arguments.type = 2;
arguments.size = size;

pthread_t tid;
pthread_create(&tid, NULL, sock_create, &arguments);
sleep(1);
```

When the calling function goes out of scope, arguments will be invalid and can have undefined behaviour in the 
thread function. Better to use malloc.
