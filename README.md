# Cpp-course-2018
ITMO cpp-course hw's

## 1. Long number arithmetics in nasm(Windows x32 implementation)
  Addition, subraction and multiplication of <512-bit numbers, input and and output use WinAPI.
  
## 2-3. Long number arithmetics in C++ using copy-on-write and small-object optimizations
  Almost all common operations that are use integral types. Pretty well optimized :)
  
## 4. File compress tool based on Huffman compressing algorithm
  Command-line tool for compressing files of any size (even if they are too big for RAM). 
  
Interface:  
`pack <src>[to <dst>]`  
`unpack <src>[to <dst>]`  

Also provided library with algorithms for compressing and decompressing any stream of bits. 

## End-of-semester exam
  Implementation of STL-like list with major part of std::list's functions. Full bidirectional iterator support and strong exception warranty provided.

## 5. GUI tool for detecting equal files in your filesystem.  
  This quite useful tool can pretty-print your directory content and suggest you to remove duplicates.  
Written with Qt framework.

## 6. Librarian - tool for searching string in text files
  Librarian is what you need if you don't like command-line tools like grep. It can index your filesystem and rapidly find all files that contain your desired string.  
Written with Qt(5.11) framework.
