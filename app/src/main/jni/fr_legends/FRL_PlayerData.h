#pragma once

// ============================================================
// FR Legends — PlayerData hooks
// ============================================================

// ---- Buy Car Slot Gold = 0 ----
bool IsGetBuySlotGold = false;

int (*old_GetBuySlotGold)(void* instance, void* method_info);
int GetBuySlotGold(void* instance, void* method_info) {
    if (instance != nullptr) {
        if (IsGetBuySlotGold) {
            return 0;
        }
    }
    return old_GetBuySlotGold(instance, method_info);
}
