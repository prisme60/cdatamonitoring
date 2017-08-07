# cdatamonitoring
Data monitoring written in C language (Data are read from the kernel via ioctl, then store to several circular buffers, each circular buffer correspond to a sampling period, recent data (several samples per minutes), old data (24 samples / day)

# Build status
- Travis : [![Build Status](https://travis-ci.org/prisme60/cdatamonitoring.svg?branch=master)](https://travis-ci.org/prisme60/cdatamonitoring)
