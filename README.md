# SignalGuardian

Open `SignalGuardian.slnx` in Visual Studio 2026 with the Desktop development with C++ workload.
Select **Debug | x64** and press **Ctrl+F5** to run the C17 FreeRTOS demo.

The solution contains one console application. Its three sender tasks and receiver
use the Windows simulator port and three FreeRTOS queues. Expected message IDs
are **4, 5, 3, 1, 2** (urgent, system, then general). The program exits after
printing the burst.

The project includes the sources and headers it uses from the FreeRTOS V10.6.2
kernel shipped with the FreeRTOS book Windows simulator examples. See
[the kernel license](SignalGuardian/FreeRTOS-Kernel/LICENSE.md).
