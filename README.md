# FTP Server and client

### Overview
This was a course project where teams build an FTP server and client supporting basic functionality. Each team would then try to exploit intentional vulnerabilities left behind in other teams' code and then patch the bugs to finish it off.

### Functions handled:
1. Handles multiple clients through [clone(2)](http://man7.org/linux/man-pages/man2/clone.2.html). (Can also be modified to use threads).
2. Multiple parallel file **GET** and **PUT** requests through use of threads.
3. Commands like **login**, **ls**, **cd**, **echo**, **ping** etc...
4. Capability of adding any other command by specifying alias in config file of server.
5. Supports reading commands from an input file and saving output to an output file.

### Code structure

* **src**: Contains server and client C++ code.
* **include**: Contains header file including common functionality of server and client.

### Running

1. Run `make` in the home folder.
2. A bin folder containing the executables is created.
3. Server is run without any arguments. It reads configuration from sploit.config
4. Clinet is run as :`./client [ServerIP] [ServerPORT]`
5. Automated client is run as : `./client [ServerIP] [ServerPORT] [InputFile] [OutputFile]`
6. Format required for command executions can be found in inputfiles of phase2.

### Extra files

Reports, Phase2, Phase3 include the bug hunting and patching phases.
