#!/bin/bash

code="$PWD"
opts=-g
cd o > /dev/null
g++ $opts $code/g -o b
cd $code > /dev/null
