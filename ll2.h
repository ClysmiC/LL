#ifndef ALS_LL2_H
#define ALS_LL2_H

//
// Macros for creating and working with intrusive doubly linked lists
//
// Requires:
//  -offsetof macro
//  -uintptr_t
//
// Usage example:
#if 0
struct Entity
{
    // This defines a node struct that has prev and next pointers
    DefineLL2(Entity);
    
    // Lists of active/inactive entities are maintained as intrusive
    //  linked lists
    LL2Node listActive;
    LL2Node listInactive;
};

struct EntityManager
{
    // These are the lists themselves. They simply hold a pointer to the head, and create
    //  a compile-time constant for the offset within an Entity
    DeclareLL2(Entity, listActive) activeEntities;
    DeclareLL2(Entity, listInactive) inactiveEntities;
};

void ActivateEntity(EntityManager * manager, Entity * entity)
{
    // NOTE - Removing from list that we aren't a member of or adding to a list that we are
    //  already a member of are both no-ops
    LL2Remove(Entity, manager->inactiveEntities, entity);
    LL2Add(Entity, manager->activeEntities, entity);
}

void DeactivateEntity(EntityManager * manager, Entity * entity)
{
    LL2Remove(Entity, manager->activeEntities, entity);
    LL2Add(Entity, manager->inactiveEntities, entity);
}

void ToggleEntityActive_Example1(EntityManager * manager, Entity * entity)
{
    if (LL2Contains(Entity, manager->activeEntities, entity))
    {
        DeactivateEntity(manager, entity);
    }
    else
    {
        ActivateEntity(manager, entity);
    }
}

void ToggleEntityActive_Example2(EntityManager * manager, Entity * entity)
{
    // Slightly more contrived implementation to demonstrate LL2Ref

    Entity::LL2Ref listToAddTo;
    Entity::LL2Ref listToRemoveFrom;

    if (LL2Contains(Entity, manager->activeEntities, entity))
    {
        LL2MakeRef(&listToAddTo, manager->inactiveEntities);
        LL2MakeRef(&listToRemoveFrom, manager->activeEntities);
    }
    else
    {
        LL2MakeRef(&listToAddTo, manager->activeEntities);
        LL2MakeRef(&listToRemoveFrom, manager->inactiveEntities);
    }

    LL2RefRemove(Entity, listToRemoveFrom, entity);
    LL2RefAdd(Entity, listToAddTo, entity);
}
#endif

//
//
//
//
//
//
//
//
//
//

#define DefineLL2(type)                         \
    struct LL2Node                              \
    {                                           \
        type * prev;                            \
        type * next;                            \
    };                                          \
    struct LL2Ref                               \
    {                                           \
        type ** ppHead;                         \
        uintptr_t offset;                       \
    }
    

        
#define DeclareLL2(type, linkMember)                                    \
        struct LL2_##linkMember                                         \
        {                                                               \
            type * head;                                                \
            static const uintptr_t offset = offsetof(type, linkMember); \
        };                                                              \
        LL2_##linkMember


// NOTE - DeclareLL2 makes a unique type for each list, and stores the offset as a compile time constant.
//  If you want to operate on a list that you select at runtime, you can do so by making an LL2Ref
#define LL2MakeRef(listRefPtr, list)            \
    do {                                        \
        (listRefPtr)->ppHead = &list.head;      \
        (listRefPtr)->offset = list.offset;     \
    } while(0)

// NOTE - Null indicates an empty list. For non-empty lists, the end is indicated by a
//  sentinel value. This lets items trivially check if they are members of a list by
//  checking for null. (The sentinel value is required to make this work if they are
//  the only member of the list!)
#define LL2EndOfList 0x1


#define LL2NodePtr_(type, itemPtr, listOffset)                  \
    ((type::LL2Node *)((unsigned char * )itemPtr + listOffset))

#define LL2NodePtr(type, list, itemPtr)         \
    LL2NodePtr_(type, itemPtr, list.offset)

#define LL2RefNodePtr(type, listRef, itemPtr)   \
    LL2NodePtr_(type, itemPtr, listRef.offset)



#define LL2Contains_(type, itemPtr, listOffset)                 \
    (LL2NodePtr_(type, itemPtr, listOffset)->prev != nullptr)

#define LL2Contains(type, list, itemPtr)        \
    LL2Contains_(type, itemPtr, list.offset)

#define LL2RefContains(type, listRef, itemPtr)  \
    LL2Contains(type, itemPtr, listRef.offset)



#define LL2Add_(type, listHeadPtr, listOffset, itemPtr)                 \
    do {                                                                \
        if (LL2Contains_(type, itemPtr, listOffset)) break;             \
        auto * itemNode_ = LL2NodePtr_(type, itemPtr, listOffset);      \
        if (*listHeadPtr)                                               \
        {                                                               \
            LL2NodePtr_(type, *listHeadPtr, listOffset)->prev = itemPtr; \
            itemNode_->next = *listHeadPtr;                             \
        }                                                               \
        else                                                            \
        {                                                               \
            itemNode_->next = (type *)LL2EndOfList;                     \
        }                                                               \
        itemNode_->prev = (type *)LL2EndOfList;                         \
        *listHeadPtr = itemPtr;                                         \
    } while(0)

#define LL2Add(type, list, itemPtr)                 \
    LL2Add_(type, &list.head, list.offset, itemPtr)

#define LL2RefAdd(type, listRef, itemPtr)                   \
    LL2Add_(type, listRef.ppHead, listRef.offset, itemPtr)


#define LL2Remove_(type, listHeadPtr, listOffset, itemPtr)      \
    do {                                                        \
        auto * node_ = LL2NodePtr_(type, itemPtr, listOffset);  \
        type * prev_ = node_->prev;                             \
        type * next_ = node_->next;                             \
        if (!prev_) break;                                      \
        bool hasNext_ = next_ != (type *)LL2EndOfList;          \
        bool hasPrev_ = prev_ != (type *)LL2EndOfList;          \
        if (hasNext_)                                           \
        {                                                       \
            LL2NodePtr_(type, next_, listOffset)->prev = prev_; \
        }                                                       \
        if (hasPrev_)                                           \
        {                                                       \
            LL2NodePtr_(type, prev_, listOffset)->next = next_; \
        }                                                       \
        else                                                    \
        {                                                       \
            if (hasNext_)                                       \
            {                                                   \
                *listHeadPtr = next_;                           \
            }                                                   \
            else                                                \
            {                                                   \
                *listHeadPtr = nullptr;                         \
            }                                                   \
        }                                                       \
        node_->next = nullptr;                                  \
        node_->prev = nullptr;                                  \
    } while(0)

#define LL2Remove(type, list, itemPtr)                  \
    LL2Remove_(type, &list.head, list.offset, itemPtr)

#define LL2RefRemove(type, listRef, itemPtr)                    \
    LL2Remove_(type, listRef.ppHead, listRef.offset, itemPtr)



#define LL2RemoveHead_(type, listHeadPtr, listOffset)                   \
    do {                                                                \
        type * headToRemove_ = *listHeadPtr;                            \
        if (headToRemove_)                                              \
        {                                                               \
            auto * headToRemoveNode_ = LL2NodePtr_(type, headToRemove_, listOffset); \
            if (headToRemoveNode_->next != (type *)LL2EndOfList)        \
            {                                                           \
                *listHeadPtr = headNode->next_;                         \
            }                                                           \
            else                                                        \
            {                                                           \
                *listHeadPtr = nullptr;                                 \
            }                                                           \
            headToRemoveNode_->next = nullptr;                          \
            headToRemoveNode_->prev = nullptr;                          \
        }                                                               \
    } while (0)

#define LL2RemoveHead(type, list)                   \
    LL2RemoveHead_(type, &list.head, list.offset)

#define LL2RefRemoveHead(type, listRef)                     \
    LL2RemoveHead_(type, listRef.ppHead, listRef.offset)



#define LL2Next_(type, itemPtr, listOffset)                             \
    ((LL2NodePtr_(type, itemPtr, listOffset)->next == (type *)LL2EndOfList) ? nullptr : LL2NodePtr_(type, itemPtr, listOffset)->next)

#define LL2Next(type, list, itemPtr)            \
    LL2Next_(type, itemPtr, list.offset)

#define LL2RefNext(type, listRef, itemPtr)      \
    LL2Next_(type, itemPtr, listRef.offset)



#define LL2Prev_(type, itemPtr, listOffset)                             \
    ((LL2NodePtr_(type, itemPtr, listOffset)->prev == (type *)LL2EndOfList) ? nullptr : LL2NodePtr_(type, itemPtr, listOffset)->prev)

#define LL2Prev(type, list, itemPtr)            \
    LL2Prev_(type, itemPtr, list.offset)

#define LL2RefPrev(type, listRef, itemPtr)      \
    LL2Prev_(type, itemPtr, listRef.offset)

    

#define LL2Clear_(type, listHeadPtr, listOffset)                        \
    do {                                                                \
        while (*listHeadPtr)                                            \
        {                                                               \
            auto * headNode_ = LL2NodePtr_(type, *listHeadPtr, listOffset); \
            auto * headNext_ = LL2Next_(type, *listHeadPtr, listOffset); \
            headNode_->next = nullptr;                                  \
            headNode_->prev = nullptr;                                  \
            *listHeadPtr = headNext_;                                   \
        }                                                               \
    } while(0)

#define LL2Clear(type, list)                    \
    LL2Clear_(type, &list.head, list.offset)

#define LL2RefClear(type, listRef)                  \
    LL2Clear_(type, listRef.ppHead, listRef.offset)



#define LL2IsEmpty_(listHeadPtr)                \
    (!(*listHeadPtr))

#define LL2IsEmpty(list)                        \
    LL2IsEmpty_(&list.head)

#define LL2RefIsEmpty(listRef)                  \
    LL2IsEmpty_(listRef.ppHead)



#define ForLL2_(type, it, listHeadPtr, listOffset)                      \
    for (type * it = *listHeadPtr; it; it = LL2Next_(type, it, listOffset))
    
#define ForLL2(type, it, list)                  \
    ForLL2_(type, it, &list.head, list.offset)

#define ForLL2Ref(type, it, listRef)                    \
    ForLL2_(type, it, listRef.ppHead, listRef.offset)



//
// Author: Andrew Smith - alsmith.net
// License: MIT
//
#endif
