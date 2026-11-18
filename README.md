# Signal Guardians

A message creator, categorizer, and priority scheduling policy written in C++17.
The host examples have no FreeRTOS dependency. An optional integration header
provides task priorities for the team's future FreeRTOS application.

This branch contains Le Huu Duy's message and scheduling module for
[PriorityLink by Team SignalGuardians](https://github.com/naomesk/signalguardians).
The team overview calls GENERAL messages Normal and URGENT messages Emergency.

| Type | Application-defined priority | Example |
| --- | --- | --- |
| `MessageType::GENERAL` | 1 - Low | Temperature: 25C |
| `MessageType::SYSTEM` | 2 - Medium | Communication restarted |
| `MessageType::URGENT` | 3 - High | Oxygen level critical! |

`enum class MessageType` prevents string typos and does not implicitly convert to an integer.
`Message` contains `id`, `type`, `char payload[128]`, and `retryCount`; it has no timestamp yet.

## Creating and categorizing messages

```cpp
#include "signalguardians/message.hpp"

using namespace signalguardians;

MessageCreator creator;
Message message = creator.create(MessageType::URGENT, "Oxygen level critical!");

auto name = MessageCategorizer::getTypeName(message.type); // "URGENT"
int priority = MessageCategorizer::getPriority(message.type); // 3
```

- `MessageCreator` assigns sequential IDs starting at 1 and sets `retryCount = 0`.
  Share one creator throughout the application to avoid duplicate IDs. IDs do not
  persist across runs. The creator does not support concurrent calls; synchronize
  access when integrating multiple tasks.
- A payload holds up to 127 bytes of content and a terminating `\0` byte. The limit
  counts bytes, not Unicode characters. Oversized content throws `std::length_error`;
  an invalid type or embedded `\0` byte throws `std::invalid_argument`.
  Exhausting the `int` ID range throws `std::overflow_error`.
- `MessageCategorizer` returns the type name and priority. Invalid types produce
  `"UNKNOWN"` and priority 0.
- Priority is an application rule, independent of FreeRTOS task priorities.
  The original example in `src/main.cpp` prints messages in creation order.

## Priority scheduling

`selectNextMessageType()` in `include/signalguardians/scheduling.hpp` chooses
URGENT, then SYSTEM, then GENERAL from a synchronized snapshot of pending counts.
It returns `std::nullopt` when no work is pending. Queue storage, FIFO within a type,
and synchronization belong to the queue module.

`TaskPriorities` defines the initial single-core FreeRTOS task priorities:
urgent sender 4, system sender and retry 3, receiver 2, general sender and logger 1.
`freertos_task_priorities.hpp` exposes the same values as `UBaseType_t` and rejects
incompatible scheduling configurations at compile time. Include it only in a
configured FreeRTOS build.

See [the scheduling design](docs/design/scheduling.md) for rules, task handoff,
configuration, starvation limitations, and the remaining runtime acceptance work.
The two Week 12 tasks are tracked there against the supplied Notion screenshots;
the live Notion board has not been updated.

## Building with g++

Run from the project directory in PowerShell:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/message.cpp src/main.cpp -o build/signalguardians.exe
./build/signalguardians.exe
```

Expected output:

```text
ID: 1 | Type: GENERAL | Message: Temperature: 25C
ID: 2 | Type: URGENT | Message: Oxygen level critical!
ID: 3 | Type: SYSTEM | Message: Communication restarted
```

Run the tests:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/message.cpp tests/message_tests.cpp -o build/message_tests.exe
./build/message_tests.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/scheduling.cpp tests/scheduling_tests.cpp -o build/scheduling_tests.exe
./build/scheduling_tests.exe
```

Run the host priority demonstration:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude src/message.cpp src/scheduling.cpp examples/priority_demo.cpp -o build/priority_demo.exe
./build/priority_demo.exe
```

It creates two GENERAL messages, one SYSTEM message, and two URGENT messages.
Dispatch order is IDs **4, 5, 3, 1, 2**: urgent first, with FIFO within each type.
This demonstration uses host containers; it does not run a FreeRTOS scheduler.

Alternatively, use CMake if installed:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

For multi-configuration generators, add `--config Debug` when building and `-C Debug` when running CTest.

`Message` has a fixed size and is trivially copyable, making it suitable as a queue
item when integrating FreeRTOS. The creator currently uses C++ exceptions; adapt
error reporting if the firmware disables exceptions.
