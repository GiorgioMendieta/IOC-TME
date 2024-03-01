#!/usr/bin/env python
import os, time

# Open Server to Fake FIFO
s2fName = '/tmp/s2f_fw'
if not os.path.exists(s2fName):
   os.mkfifo(s2fName)
s2f = open(s2fName,'w') # 'w+'

# Open Fake to Server FIFO
f2sName = '/tmp/f2s_fw'
if not os.path.exists(f2sName):
   os.mkfifo(f2sName)
f2s = open(f2sName,'r')

# Write on the s2f FIFO and flush it
s2f.write("w hello\n")
s2f.flush()

# Read the f2s FIFO and print its contetns
str = f2s.readline()
print('%s' % str,)

f2s.close()
s2f.close()
