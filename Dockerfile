FROM ubuntu:18.04

RUN apt-get update -y && apt-get install bsdmainutils binutils gdb nasm build-essential -y
WORKDIR /usr/src/ubuntu

