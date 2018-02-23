#!/bin/sh

g++ -o server server.cpp include/sploit.cpp -lpthread -std=c++11
g++ -o client client.cpp include/sploit.cpp -lpthread -std=c++11
