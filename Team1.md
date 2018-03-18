##Team1

### Buffer overflow

* Giving a long path to the CD command will overflow the path buffer. The code as it is, is trying to set an invalid array positon. The only catch is that, the directory which we want to cd into, should exist.

```C
char path[128];
// check size limitations
size_t len = strlen(current);
strncpy(path, current, len);
path[128] = '\0';
```
```
Server output:
 [127.0.0.1] Received line: login n
 [127.0.0.1] Received line: pass 1
 [127.0.0.1] Successfully logged in as: n
 [127.0.0.1] Received line: cd AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Segmentation fault (core dumped)
mc08 56 $ 

```


### Design bugs

* None of the alias commands take parameters.
* No directory structure


