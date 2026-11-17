# Signal Guardians

A message creator and categorizer written in C++17, with no FreeRTOS dependency.

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
  The example prints messages in creation order; it does not process them by priority.

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
```

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
