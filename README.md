# mini-coreutils

A lightweight reimplementation of classic **GNU coreutils**, built from scratch using **low-level syscalls** and **manual system interactions**.
This project aims to replicate essential tools like `ls`, `cat`, `mkdir`, `mktemp`, etc — one command at a time — without relying on high-level libc abstractions.

---

## 🚀 Goal
The objective is to understand **how core utilities work internally**, handling files, directories, memory and IO with **raw system calls**

---

## 🛠️ Build & Usage
```bash
# clone repo
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

## 📅 Development Status

Work in progress — one tool at a time, no rush, maximum learning.
Help, PRs and flames welcome

---

## 📜 MIT License

Free to break, modify and burn however you wish.

---
