#include "signalguardians/scheduling.h"

bool select_next_message_type(PendingMessageCounts waiting, MessageType *next)
{
    if (waiting.urgent > 0) {
        *next = MESSAGE_URGENT;
    } else if (waiting.system > 0) {
        *next = MESSAGE_SYSTEM;
    } else if (waiting.general > 0) {
        *next = MESSAGE_GENERAL;
    } else {
        return false;
    }
    return true;
}
