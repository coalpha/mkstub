# mkstub

Sometimes symlinking to a binary doesn't work since the binary uses `argv[0]`
to derive the executable's location. That's why we have stubs.

I wanted a way of creating stubs for executables that didn't involve writing a
batch script. Not only do I not know how to write batch scripts but I'm willing
to bet that this is a lot faster.

First, we start by making a stub for a single application such as the venerable
`calc.exe`. Reviewing the binary, we can clearly see exactly where the
executable path (in this case `C:\Windows\System32\calc.exe`) lies.

```
000005d0  00 00 00 00 00 00 00 00  e0 01 47 65 74 43 6f 6d  |..........GetCom|
000005e0  6d 61 6e 64 4c 69 6e 65  57 00 da 02 47 65 74 53  |mandLineW...GetS|
000005f0  74 61 72 74 75 70 49 6e  66 6f 57 00 e8 00 43 72  |tartupInfoW...Cr|
00000600  65 61 74 65 50 72 6f 63  65 73 73 57 00 00 6a 02  |eateProcessW..j.|
00000610  47 65 74 4c 61 73 74 45  72 72 6f 72 00 00 67 01  |GetLastError..g.|
00000620  45 78 69 74 50 72 6f 63  65 73 73 00 ea 05 57 61  |ExitProcess...Wa|
00000630  69 74 46 6f 72 53 69 6e  67 6c 65 4f 62 6a 65 63  |itForSingleObjec|
00000640  74 00 46 02 47 65 74 45  78 69 74 43 6f 64 65 50  |t.F.GetExitCodeP|
00000650  72 6f 63 65 73 73 00 00  89 00 43 6c 6f 73 65 48  |rocess....CloseH|
00000660  61 6e 64 6c 65 00 4b 45  52 4e 45 4c 33 32 2e 64  |andle.KERNEL32.d|
00000670  6c 6c 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |ll..............|
00000680  1c 00 00 00 00 00 00 00  43 00 3a 00 5c 00 57 00  |........C.:.\.W.|
00000690  69 00 6e 00 64 00 6f 00  77 00 73 00 5c 00 53 00  |i.n.d.o.w.s.\.S.|
000006a0  79 00 73 00 74 00 65 00  6d 00 33 00 32 00 5c 00  |y.s.t.e.m.3.2.\.|
000006b0  63 00 61 00 6c 00 63 00  2e 00 65 00 78 00 65 00  |c.a.l.c...e.x.e.|
```

What if we were to simply patch the string as it sits in the binary instead of
recompiling for different target executable paths? Well, that's exactly what's
done here.

`template.c` is, as it says on the tin, a template stub. First, we allocate a
bunch of space for a counted string.

```c
size_t const coalphaa = 0x616168706c616f63; 
struct counted_wstr {
   size_t length;
   WCHAR chars[];
};
counted_wstr exe_path = {
   .length = coalphaa,
   .chars = {[PATH_LIMIT] = 0},
};
```

Why is `.length` set to `coalphaa`? Well, as you'll see in the compiled output...

```
00000680  63 6f 61 6c 70 68 61 61  00 00 00 00 00 00 00 00  |coalphaa........|
```

This makes it quite simple to search for the beginning of the `struct counted_wstr`.
Since we can easily find where `exe_path` is stored, let's import the
`template.exe` binary into `mkstub.c` using `xxd(1)`. Now all that needs to be
done is getting input from the user and writing the patched binary to disk.
