#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 sw=4 expandtab:

"""
完成作业以后，运行本文件，它会将你的代码、实验报告一起打包为 submit.zip。
"""

from __future__ import print_function # Requires Python 2.7+

import locale
import os
import re
import sys
import zipfile


# Python3's input == Python2's raw_input
try:
    input_compat = raw_input
except NameError:
    input_compat = input


def S(s):
    # F*** systems that still refuse to use UTF-8 by default in the 21st century
    if 'decode' in dir(s):
        return s.decode('utf-8').encode(locale.getpreferredencoding()) # Python 2 is naughty
    else:
        return s # Python 3 is good
    # FIXME: Can anybody please tell me whether there is a simpler way to make Chinese
    # characters display correctly in both Python 2 and 3, and in all three major OSes
    # (Win/Lin/Mac)?


def main():

    # Preparations
    locale.setlocale(locale.LC_ALL, '')

    # Check whether decaf.jar exists
    decaf_jar = os.path.join('.', 'SimJoiner.cpp')
    if not os.path.exists(decaf_jar):
        print(S('未找到文件1。请重新编译。'), file=sys.stderr)
        return 1
    print(S('已找到 {}'.format(decaf_jar)))
    decaf_jar2 = os.path.join('.', 'SimJoiner.h')
    if not os.path.exists(decaf_jar2):
        print(S('未找到文件2。请重新编译。'), file=sys.stderr)
        return 1
    print(S('已找到 {}'.format(decaf_jar2)))
    decaf_jar3 = os.path.join('.', 'Trie.cpp')
    if not os.path.exists(decaf_jar3):
        print(S('未找到文件3。请重新编译。'), file=sys.stderr)
        return 1
    print(S('已找到 {}'.format(decaf_jar3)))
    decaf_jar4 = os.path.join('.', 'Trie.h')
    if not os.path.exists(decaf_jar4):
        print(S('未找到文件4。请重新编译。'), file=sys.stderr)
        return 1
    print(S('已找到 {}'.format(decaf_jar4)))


    # Creating submit.zip
    with zipfile.ZipFile('00submit.zip', 'w') as submit_zip:
        submit_zip.write(decaf_jar, 'SimJoiner.cpp', zipfile.ZIP_STORED)
        submit_zip.write(decaf_jar2, 'SimJoiner.h', zipfile.ZIP_STORED)
        submit_zip.write(decaf_jar3, 'Trie.cpp', zipfile.ZIP_STORED)
        submit_zip.write(decaf_jar4, 'Trie.h', zipfile.ZIP_STORED)

    # Finished
    print(S('完成。请将 00submit.zip 文件上传到网络学堂。'))
    return 0


if __name__ == '__main__':
    retcode = main()
#    if os.name == 'nt':
#        input_compat('Press Enter to continue...')
    sys.exit(retcode)
