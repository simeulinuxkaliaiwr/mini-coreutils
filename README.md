# mini-coreutils

A lightweight reimplementation of classic **GNU coreutils**, built from scratch using **low-level syscalls** and **manual system interactions**.
This project aims to replicate essential tools like `ls`, `cat`, `mkdir`, `mktemp`, etc — one command at a time — without relying on high-level libc abstractions.

---

## 🚀 Goal
The objective is to understand **how `core` utilities work internally**, handling files, directories, memory and IO with **raw system calls**

---

## 🛠️ Build & Usage
```bash
# clone repo via ssh (recommended)
git clone git@github.com:simeulinuxkaliaiwr/mini-coreutils.git
# clone repo via https
git clone https://github.com/simeulinuxkaliaiwr/mini-coreutils
cd mini-coreutils

# build single utils (example: ls)
make mini-ls

# build all utils
make

# list avaliable utils
make list

# rebuild all utils
make rebuild

# remove /bin and /obj
make clean
```

---

## 🌟 Tools that are currently available

* mini-ls - List directory contents
* mini-cat - Concatenate and display files
* mini-echo - Display a line of text
* mini-pwd - Print working directory

---

## 📋 Planned utilities (In order of implementation)

1. mini-mkdir - Make directories
2. mini-rm - Remove files/directories
3. mini-cp - Copy files
4. mini-mv - Move/rename files
5. mini-mktemp - Create a temporary file or directory

---

## 📅 Development Status

Work in progress — one tool at a time, no rush, maximum learning.
Help, PRs and flames welcome

---

## 📜 MIT License

Free to break, modify and burn however you wish.

---
