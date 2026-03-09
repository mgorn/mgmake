#!/usr/bin/env bash
# Use this script to build & run the build script
clang++ -o builder -I include -std=c++2c build.cxx
./builder