#!/bin/bash
# $1 = path to the AFL test case file
# Read file contents and pass as argv[1] to buggy program
./buggy "$(cat "$1")"
