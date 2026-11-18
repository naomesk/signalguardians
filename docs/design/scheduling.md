# Message types and priority scheduling

Owner: Le Huu Duy. This document covers the two Week 12 tasks shown in the
Team To-Do List, due November 23, 2026:

- Define message structure and the three message types.
- Set FreeRTOS task priorities and priority scheduling rules.

The screenshots show both tasks as In progress. This repository records local
implementation evidence; it does not change their Notion status.

## Shared message contract

`include/signalguardians/message.hpp` is the shared C++17 definition.

| Field | Type | Rule |
| --- | --- | --- |
| `id` | `int` | Positive, sequential within one shared creator; not persistent |
| `type` | `MessageType` | GENERAL, SYSTEM, or URGENT |
| `payload` | `char[128]` | Up to 127 content bytes followed by a null terminator |
| `retryCount` | `int` | Starts at zero; updated by the retry module |

The type is fixed-size, standard-layout, and trivially copyable. Compile-time
checks protect those properties. It contains no owning pointers. FreeRTOS queue
creation must use `sizeof(Message)`; do not assume a packed byte size or use the
in-memory representation as a radio/wire format. Padding and integer layout are
toolchain-dependent. Timestamp and encrypted-payload representation remain future
integration work. The current creator accepts text, not arbitrary ciphertext.

Keep the existing C++ names so callers continue to work:

| Team overview name | C++ type | Message rank |
| --- | --- | --- |
| Normal | `MessageType::GENERAL` | 1 |
| System | `MessageType::SYSTEM` | 2 |
| Emergency | `MessageType::URGENT` | 3 |

Ranks are explicitly mapped by `MessageCategorizer::getPriority`; they are not
FreeRTOS task priorities. Invalid types return rank 0 and name UNKNOWN, and the
creator rejects them. Use one creator and serialize its calls across producers.
The current creator throws C++ exceptions on invalid input; an exception-disabled
firmware build will need an agreed error-return interface.

## Initial task priority assignments

`TaskPriorities` in `scheduling.hpp` centralizes these initial values. They are
design choices for the first integrated experiment, not measured optimum values.

| Task | Absolute FreeRTOS priority | Reason |
| --- | --- | --- |
| Urgent / Emergency sender | 4 | Admit urgent traffic even while the receiver drains a backlog |
| System sender | 3 | Admit subsystem status before more queued routine work is processed |
| Retry | 3 | Make due retries available promptly, then block until more work is due |
| Receiver | 2 | Drain admitted work ahead of routine generation; block when empty |
| General / Normal sender | 1 | Generate background telemetry at a bounded rate |
| Logger | 1 | Process logs without preempting delivery work |
| Kernel idle | 0 | Reserved for the FreeRTOS idle task |

The timer daemon, if enabled, belongs to the target's FreeRTOS configuration and
must be considered during integration. These values are not interrupt priorities.

Urgent and system senders outrank the receiver so their arrivals can be admitted
while it processes a general-message backlog. If the receiver outranked those
senders and stayed ready, even a priority-aware queue could not select messages
that the senders had not yet had CPU time to create. Producers and retry work must
still be bounded and block between events; continuous high-priority production
can starve delivery itself.

Merge the following scheduling settings into the target's existing
`FreeRTOSConfig.h`; this is not a complete port configuration:

```c
#define configUSE_PREEMPTION   1
#define configUSE_TIME_SLICING 1
#define configMAX_PRIORITIES  5
```

More than five priority levels is also valid. If the target exposes
`configNUMBER_OF_CORES`, use 1 for this initial policy. Equal-priority ready tasks
share time slices; lower-priority ready tasks run only when higher-priority work
blocks or otherwise becomes unready. A continuously ready high-priority task can
starve lower priorities. `taskYIELD()` alone does not make a lower-priority task run.

Use `freertos_task_priorities.hpp` only from the configured FreeRTOS application.
It checks preemption, time slicing, the available priority range, and the single-core
assumption at compile time, and exposes constants of type `UBaseType_t`.

For example, in the receiver owner's task initialization code, using that module's
actual function, stack depth, context, and handle:

```cpp
#include "signalguardians/freertos_task_priorities.hpp"

const BaseType_t result = xTaskCreate(
    receiverTask, "Receiver", receiverStackDepth, receiverContext,
    signalguardians::FreeRtosTaskPriorities::Receiver, &receiverHandle);
// Handle result != pdPASS in the application's startup error path.
```

Every task owner must pass its matching constant to `xTaskCreate` or
`xTaskCreateStatic`. The adapter does not create tasks or alter existing handles.

## Message dispatch rules

At each new dispatch decision:

1. Choose URGENT if any urgent messages are pending.
2. Otherwise choose SYSTEM if any system messages are pending.
3. Otherwise choose GENERAL if any general messages are pending.
4. Otherwise report no work and block on an arrival event.
5. Preserve enqueue order within each type (FIFO).

`selectNextMessageType(PendingMessageCounts)` implements rules 1-4 and returns
`std::nullopt` for empty queues. The queue module owns rule 5, synchronization,
storage limits, and admission/overflow handling. Selecting a message does not
interrupt a message already being delivered; re-evaluate after that delivery
attempt. Urgent arrival must be considered at the next synchronized selection.

A single FreeRTOS FIFO does not sort its entries by `Message::type`. Increasing a
sender's task priority does not reorder an existing backlog. The queue owner can
provide three FIFO lanes behind one interface, or another bounded priority-aware
storage implementation. The count snapshot, selection, and removal must form one
synchronized operation relative to producers. Do not hold a mutex or critical
section while waiting for work, processing a message, or doing link I/O.

The host example in `examples/priority_demo.cpp` illustrates three FIFO lanes with
`std::deque`. Those containers are a host demonstration, not the embedded queue
implementation. With creation order GENERAL, GENERAL, SYSTEM, URGENT, URGENT,
dispatch IDs are 4, 5, 3, 1, 2.

This is strict priority with no aging. GENERAL and SYSTEM can wait indefinitely
under a sustained urgent load. Do not claim starvation freedom. The Week 14 tuning
work should compare measured urgent latency, lower-priority wait times, queue
depth, and overflow before changing the policy. These initial values do not prove
the team's proposed 10 ms latency target.

## Integration responsibilities

- Queue owner: priority-aware selection/removal, FIFO per type, bounded capacity,
  and synchronized wakeups with no missed-arrival race. Define admission failures
  explicitly, including reserved urgent capacity if the team chooses it.
- Sender/receiver owner: use the priority constants, serialize creator access,
  rate-limit producers, and block the receiver on a reliable work notification.
  A low-priority transmission must have a bounded duration so urgent work can be
  considered promptly at the next dispatch boundary.
- Retry owner: route due messages through the same priority policy; preserve ID
  and type, increment retry count, and block between due times. Immediate urgent
  retry and retry limits still need implementation in that module.
- Logger/testing owner: collect timestamps and queue metrics. Logging from higher
  priority tasks must not perform unbounded synchronous output or wait indefinitely
  on a full logging queue. Define overflow reporting.

## Validation and remaining work

Host tests cover the message contract, all eight queue-availability combinations,
a late urgent arrival against 100 general messages, recovery to lower classes
after higher classes drain, and empty/large-count cases. The demo shows FIFO order
within a type. Host checks do not execute the FreeRTOS scheduler.

The compile-only check in `tests/freertos_policy_compile.cpp` also passed against
the real FreeRTOS kernel headers at commit
[`ca759444e9ca59cfa3a9faa6cb8b49e0772ca520`](https://github.com/FreeRTOS/FreeRTOS-Kernel/commit/ca759444e9ca59cfa3a9faa6cb8b49e0772ca520),
using the MSVC-MingW port headers and a minimal compilation configuration.
Configurations with four priority levels, preemption disabled, time slicing
disabled, or two cores were each rejected as expected. These are header/API and
configuration checks, not a decision to use the Windows simulator.

To repeat the compile-only check with a local kernel checkout and the team's
actual configuration, set `$kernelRoot` and `$configDir` to those directories and
use the port include path for the chosen target. For MSVC-MingW with g++:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -Iinclude "-I$kernelRoot/include" "-I$kernelRoot/portable/MSVC-MingW" "-I$configDir" tests/freertos_policy_compile.cpp
```

The simulator/target is not yet decided, as confirmed by Le Huu Duy. No kernel
runtime, latency, starvation, or concurrent queue experiment has been run here.

Before completing the FreeRTOS scheduling task in Notion:

- Choose the simulator/port and merge these priorities into the real task creation.
- Connect the queue implementation to the dispatch policy and verify wakeups under
  concurrent producers.
- Run on the kernel: check urgent-before-general among pending messages, equal-type
  FIFO, receiver blocking, and task-priority readbacks.
- Measure latency and lower-priority progress under bounded and sustained load;
  record overflow, stack usage, and logger behavior.

## Sources

- Task names, owner, due date, and Notion status: screenshots supplied by Le Huu Duy.
- [Team project overview](https://github.com/naomesk/signalguardians/blob/main/README.md).
- [FreeRTOS task priorities](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities).
- [FreeRTOS scheduling](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/04-Task-scheduling).
- [FreeRTOS queues](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues).
