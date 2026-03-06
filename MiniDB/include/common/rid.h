//
//  rid.h
//  MiniDB
//
//  Created on 2026/1/10.
//

#ifndef RID_H
#define RID_H

#include <functional>

// RID: Record Identifier（记录标识符）
struct RID {
    int page_id;    // 页面ID
    int slot_num;   // 槽位号（在页面中的位置）
    
    RID() : page_id(-1), slot_num(-1) {}
    
    RID(int page_id, int slot_num) 
        : page_id(page_id), slot_num(slot_num) {}
    
    bool operator==(const RID &other) const {
        return page_id == other.page_id && slot_num == other.slot_num;
    }
    
    bool operator!=(const RID &other) const {
        return !(*this == other);
    }
    
    bool IsValid() const {
        return page_id >= 0 && slot_num >= 0;
    }
};

// 为 RID 提供哈希函数，以便在 unordered_map/unordered_set 中使用
namespace std {
    template<>
    struct hash<RID> {
        size_t operator()(const RID& rid) const {
            return std::hash<int>()(rid.page_id) ^ (std::hash<int>()(rid.slot_num) << 1);
        }
    };
}

#endif 
