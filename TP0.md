TP0: Introducción a JOS
=======================

backtrace_func_names
--------------------

Salida del comando `backtrace`:

```
K> backtrace
  ebp f010ff48  eip f0100a25  args 00000001 f010ff70 00000000 0000000a 00000009
         kern/monitor.c:118: runcmd+225
  ebp f010ffc8  eip f0100a90  args 00010094 00010094 f010fff8 f01000e1 00000000
         kern/monitor.c:136: monitor+62
  ebp f010ffd8  eip f01000e1  args 00000000 00001aac 00000644 00000000 00000000
         kern/init.c:43: i386_init+77
  ebp f010fff8  eip f010003e  args 00111021 00000000 00000000 00000000 00000000
         kern/entry.S:83: <unknown>+0

```
