SofieOS: Memory-safe Real-time Operating System for Low-cost Embedded Devices
===

Introduction
---
This is the repository of source code of SofieOS: Memory-safe Real-time Operating System for Low-cost Embedded Devices, which is submitted to OSDI 2023 (paper #196).

Environment Setting
---
To buld and run SofieOS, you need to download FreedomStudio and unzip it to D:\FreedomStudio drive on windows. You can also unzip to other location and manually set the location in .settings/freedomstudio.preference.prefs.

Then you can open FreedomStudio, and click File/Open Projects from File System,
then choose the directory of this repository.

You also need to buy a Sifive Hifive1-revb board to run SofieOS.

You can download FreedomStudio at: https://www.sifive.com/software, and you can get a Sifive Hifive1-revb board at: https://www.sifive.com/boards/hifive1-rev-b.

Structure
---
```
.settings: Project configuration files of FreedomStudio
bsp: linker scripts
freedom-metal: FreedomSDK
kernel: SofieOS kernel
scripts: some scripts needs to build SofieOS
src: Source code of user tasks (now are benchmarks)
```

Usage
---
You need to follow the instructions mentioned in the paper to develop your application on SofieOS.

The entry of the first task should be `int real_main()`.

You may also need to change `USER_STACK_SIZE` and `USER_HEAP_SIZE` in `kernel/macros.h`.

To run benchmark
---
We developed many benchmarks, you can run each of them by changing the macro at line 1 in `src/main.c`.

Note: If you want to run `BENCH_qsort` and `BENCH_LFS`, you should change `USER_HEAP_SIZE` to 1024.