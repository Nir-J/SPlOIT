#!/bin/sh

g++ -o server server.cpp include/sploit.cpp -lpthread
g++ -o client client.cpp include/sploit.cpp -lpthread
