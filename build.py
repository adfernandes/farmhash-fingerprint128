#!/usr/bin/env python

import os, shutil
import subprocess

bdir = '_build'

cxx = '/Users/andrew/Developer/Toolchains/clang+llvm-3.7.0/bin/clang++'
opt = ['-Os', '-DNDEBUG']

# Notes:
# - the peculiar flags for C++11 compliance are for compiling on Apple systems, due to the header file arrangement on them
#
extra = ['-std=c++11', '-UFARMHASH_CAN_USE_CXX11', '-DFARMHASH_LITTLE_ENDIAN=1', '-ULIKELY', '-DHAVE_BUILTIN_EXPECT=1']

archs = ['64', '32']

assume = [
    '-DFARMHASH_ASSUME_SSSE3=1',
    '-DFARMHASH_ASSUME_SSE41=1',
    '-DFARMHASH_ASSUME_SSE42=1',
    '-DFARMHASH_ASSUME_AESNI=1',
    '-DFARMHASH_ASSUME_AVX=1']
flags = [
    '-mssse3',
    '-msse4.1',
    '-msse4.2',
    '-maes',
    '-mavx']
names = [
    'ssse3',
    'sse41',
    'sse42',
    'aes',
    'avx']

assert len(assume) == len(flags)
assert len(assume) == len(names)

shutil.rmtree(bdir, ignore_errors=True)
os.mkdir(bdir)

for i in range(len(assume)):
    for arch in archs:

        cmd = [cxx]
        cmd.extend(opt)
        cmd.extend(extra)
        cmd.append('-m' + arch)

        # Note:
        # - 'clang' does not have GCC-compatible constructors for 'uint128_t', so we cannot use them
        if arch == '32':
            cmd.append('-UFARMHASH_UINT128_T_DEFINED')
        elif arch == '64':
            cmd.extend(['-Duint128_t=__uint128_t', '-DFARMHASH_UINT128_T_DEFINED=1'])

        for k in range(i+1):
            cmd.append(assume[k])
            cmd.append(flags[k])

        cmd.append('-DNAMESPACE_FOR_HASH_FUNCTIONS=farm_' + names[i])

        cmd.append('farmhash/src/farmhash.cc')

        obj = cmd[:]
        obj.extend(['-c', '-o', os.path.join(bdir, 'farmhash~' + names[i] + '~x' + arch + '.o')])
        print(' '.join(obj))
        subprocess.check_call(obj)

        asm = cmd[:]
        asm.extend(['-S', '-o', os.path.join(bdir, 'farmhash~' + names[i] + '~x' + arch + '.s')])
        print(' '.join(asm))
        subprocess.check_call(asm)
