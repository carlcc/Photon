//
// Created by carl on 20-4-3.
//

#pragma once
#include "imgui.h"
#include <string>
#include <vector>

namespace ImGui {

inline bool Combo(const char* label, int* current_item, const std::vector<std::string>& items, int height_in_items = -1)
{
    const bool value_changed = ImGui::Combo(
        label, current_item, [](void* data, int idx, const char** out_text) -> bool {
            const auto& items = *(const std::vector<std::string>*)data;
            if (out_text)
                *out_text = items[idx].c_str();
            return true;
        },
        (void*)&items, int(items.size()), height_in_items);
    return value_changed;
}

}