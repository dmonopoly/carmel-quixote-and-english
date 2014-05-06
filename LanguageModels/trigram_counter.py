#!/usr/bin/python
import sys

def main():
  file1 = sys.argv[1]
  file2 = sys.argv[2]
  fin = open(file1, 'r') # r+ means read AND write to file. 'r' is default.
  fout = open(file2, 'w')

  print '-Reading file %s-' % file1
  big_str = fin.read()
  words = big_str.split()
  count_dict = {}
  for i in range(len(words) - 2):
    triplet = (words[i], words[i+1], words[i+2])
    if triplet in count_dict:
      count_dict[triplet] += 1
    else:
      count_dict[triplet] = 1

  print '-Writing to file %s-' % file2
  for triplet in count_dict.keys():
    fout.write("%s %s %s %s" % (count_dict[triplet], triplet[0], triplet[1], triplet[2]))
    fout.write("\n")

if len(sys.argv) != 3:
  print 'Usage: python <exec> <file-to-read> <file-to-write-to>'
else:
  main()

