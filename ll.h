#ifndef ALS_LL_H
#define ALS_LL_H

#ifdef ALS_ASSERT
#define LL_ASSERT ALS_ASSERT
#else
#define LL_ASSERT()
#endif

//
// Macros for creating and working with intrusive singly (LL1) and doubly (LL2) linked lists
//
// Requires:
//  -offsetof macro
//  -uintptr_t
//
// Options:
//  -define ALS_ASSERT before including to get runtime asserts
//
//
//
//
//
//
//
//


// NOTE - Null indicates an empty list. For non-empty lists, the end is indicated by a
//  sentinel value. This lets items trivially check if they are members of a list by
//  checking for null. (The sentinel value is required to make this work if they are
//  the only member of the list!)
#define LLEndOfList_ 0x1


//
// Singly linked list
//

#define DefineLL1Node(type)                     \
    struct LL1Node                              \
    {                                           \
        struct type * pNext;                    \
    };                                          \
    struct LL1Ref                               \
    {                                           \
        struct type ** ppHead;                  \
        struct type ** ppTail;                  \
        uintptr_t offset;                       \
    };                                          \
    
#define LL1Type(type, linkMember) LL1_##type##_##linkMember
        
#define DefineLL1(type, linkMember)                                 \
    struct LL1_##type##_##linkMember                                \
    {                                                               \
        struct type * pHead;                                        \
        struct type * pTail;                                        \
        static const uintptr_t offset = offsetof(type, linkMember); \
    };



#define LL1MakeRef(listRefPtr, list)            \
    do {                                        \
        (listRefPtr)->ppHead = &list.pHead;     \
        (listRefPtr)->ppTail = &list.pTail;     \
        (listRefPtr)->offset = list.offset;     \
    } while(0)



#define LL1NodePtr_(type, pItem, listOffset)                    \
    ((type::LL1Node *)((unsigned char * )pItem + listOffset))

#define LL1NodePtr(type, list, pItem)           \
    LL1NodePtr_(type, pItem, list.offset)

#define LL1RefNodePtr(type, listRef, pItem)     \
    LL1NodePtr_(type, pItem, listRef.offset)


#define LL1IsItemLinked_(type, pItem, listOffset)               \
    (LL1NodePtr_(type, pItem, listOffset)->pNext != nullptr)

// NOTE - It's possible for an item to be linked, but not necessarily be a part of this list (i.e., there are multiple heads that all
//  use the same nodes as links and items can only be on one list). Querying this would require O(n) search.
#define LL1IsItemLinked(type, list, pItem)      \
    LL1IsItemLinked_(type, pItem, list.offset)

#define LL1RefIsItemLinked(type, listRef, pItem)    \
    LL1IsItemLinked_(type, pItem, listRef.offset)


    
#define LL1IsNodeLinked(node)                   \
        (node.pNext != nullptr)


    
#define LL1AddHead_(type, ppListHead, ppListTail, listOffset, pItem)    \
        do {                                                            \
            if (LL1IsItemLinked_(type, pItem, listOffset))              \
            {                                                           \
                LL_ASSERT(false);                                       \
                break;                                                  \
            }                                                           \
            auto * itemNode_ = LL1NodePtr_(type, pItem, listOffset);    \
            if (*ppListHead)                                            \
            {                                                           \
                itemNode_->pNext = *ppListHead;                         \
            }                                                           \
            else                                                        \
            {                                                           \
                itemNode_->pNext = (type *)LLEndOfList_;                \
                *ppListTail = pItem;                                    \
            }                                                           \
            *ppListHead = pItem;                                        \
        } while(0)

            #define LL1AddHead(type, list, pItem)                               \
                LL1AddHead_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL1RefAddHead(type, listRef, pItem)                             \
            LL1AddHead_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)

#define LL1Add(type, list, pItem)               \
            LL1AddHead(type, list, pItem)

#define LL1RefAdd(type, listRef, pItem)         \
            LL1RefAddHead(type, listRef, pItem)



#define LL1AddTail_(type, ppListHead, ppListTail, listOffset, pItem)    \
            do {                                                        \
                if (LL1IsItemLinked_(type, pItem, listOffset))          \
                {                                                       \
                    LL_ASSERT(false);                                   \
                    break;                                              \
                }                                                       \
                auto * itemNode_ = LL1NodePtr_(type, pItem, listOffset); \
                if (*ppListTail)                                        \
                {                                                       \
                    LL1NodePtr_(type, *ppListTail, listOffset)->pNext = pItem; \
                }                                                       \
                else                                                    \
                {                                                       \
                    *ppListHead = pItem;                                \
                }                                                       \
                itemNode_->pNext = (type *)LLEndOfList_;                \
                *ppListTail = pItem;                                    \
            } while(0)

                #define LL1AddTail(type, list, pItem)                               \
                    LL1AddTail_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL1RefAddTail(type, listRef, pItem)                             \
                LL1AddTail_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)



#define LL1RemoveHead_(type, ppListHead, ppListTail, listOffset, pAssignTo) \
                do {                                                    \
                    type * headToRemove_ = *ppListHead;                 \
                    pAssignTo = headToRemove_;                          \
                    if (headToRemove_)                                  \
                    {                                                   \
                        auto * headToRemoveNode_ = LL1NodePtr_(type, headToRemove_, listOffset); \
                        if (headToRemoveNode_->pNext != (type *)LLEndOfList_) \
                        {                                               \
                            *ppListHead = headToRemoveNode_->pNext;     \
                        }                                               \
                        else                                            \
                        {                                               \
                            *ppListHead = nullptr;                      \
                            *ppListTail = nullptr;                      \
                        }                                               \
                        headToRemoveNode_->pNext = nullptr;             \
                    }                                                   \
                } while (0)

                    #define LL1RemoveHead(type, list, pAssignTo)                            \
LL1RemoveHead_(type, &list.pHead, &list.pTail, list.offset, pAssignTo)

#define LL1RefRemoveHead(type, listRef, pAssignTo)                      \
    LL1RemoveHead_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pAssignTo)




#define LL1Next_(type, pItem, listOffset)                               \
    ((LL1NodePtr_(type, pItem, listOffset)->pNext == (type *)LLEndOfList_) ? nullptr : LL1NodePtr_(type, pItem, listOffset)->pNext)

#define LL1Next(type, list, pItem)              \
    LL1Next_(type, pItem, list.offset)

#define LL1RefNext(type, listRef, pItem)        \
    LL1Next_(type, pItem, listRef.offset)



#define LL1Prev_(type, pItem, listOffset)                               \
    ((LL1NodePtr_(type, pItem, listOffset)->pPrev == (type *)LLEndOfList_) ? nullptr : LL1NodePtr_(type, pItem, listOffset)->pPrev)

#define LL1Prev(type, list, pItem)              \
    LL1Prev_(type, pItem, list.offset)

#define LL1RefPrev(type, listRef, pItem)        \
    LL1Prev_(type, pItem, listRef.offset)

    

#define LL1Clear_(type, ppListHead, ppListTail, listOffset)             \
    do {                                                                \
        while (*ppListHead)                                             \
        {                                                               \
            auto * pHeadNode_ = LL1NodePtr_(type, *ppListHead, listOffset); \
            auto * pHeadNext_ = LL1Next_(type, *ppListHead, listOffset); \
            pHeadNode_->pNext = nullptr;                                \
            *ppListHead = pHeadNext_;                                   \
        }                                                               \
        *ppListTail = nullptr;                                          \
    } while(0)

#define LL1Clear(type, list)                                \
    LL1Clear_(type, &list.pHead, &list.pTail, list.offset)

#define LL1RefClear(type, listRef)                                  \
    LL1Clear_(type, listRef.ppHead, listRef.ppTail, listRef.offset)



#define LL1ClearWithoutUnlinking_(type, ppListHead, ppListTail)     \
    do { *ppListHead = nullptr; *ppListTail = nullptr; } while (0)

#define LL1ClearWithoutUnlinking(type, list)                    \
    LL1ClearWithoutUnlinking_(type, &list.pHead, &list.pTail)

#define LL1RefClearWithoutUnlinking(type, listRef)                  \
    LL1ClearWithoutUnlinking_(type, listRef.ppHead, listRef.ppTail)

    

#define LL1IsEmpty_(ppListHead)                 \
    (!(*ppListHead))

#define LL1IsEmpty(list)                        \
    LL1IsEmpty_(&list.pHead)

#define LL1RefIsEmpty(listRef)                  \
    LL1IsEmpty_(listRef.ppHead)

        

#define ForLL1_(type, it, ppListHead, listOffset)                       \
    for (type * it = *ppListHead; it; it = LL1Next_(type, it, listOffset))
    
#define ForLL1(type, it, list)                  \
    ForLL1_(type, it, &list.pHead, list.offset)

#define ForLL1Ref(type, it, listRef)                    \
    ForLL1_(type, it, listRef.ppHead, listRef.offset)



    
//
// Doubly linked list
//

#define DefineLL2Node(type)                     \
    struct LL2Node                              \
    {                                           \
        struct type * pPrev;                    \
        struct type * pNext;                    \
    };                                          \
    struct LL2Ref                               \
    {                                           \
        struct type ** ppHead;                  \
        struct type ** ppTail;                  \
        uintptr_t offset;                       \
    };                                          \
    struct LL2CombineParam                      \
    {                                           \
        struct type ** ppHead0;                 \
        struct type ** ppTail0;                 \
        struct type ** ppHead1;                 \
        struct type ** ppTail1;                 \
        uintptr_t offset;                       \
    }
    
#define LL2Type(type, linkMember) LL2_##type##_##linkMember
        
#define DefineLL2(type, linkMember)                                     \
        struct LL2_##type##_##linkMember                                \
        {                                                               \
            struct type * pHead;                                        \
            struct type * pTail;                                        \
            static const uintptr_t offset = offsetof(type, linkMember); \
        };                                                              \



#define LL2MakeRef(listRefPtr, list)            \
        do {                                    \
            (listRefPtr)->ppHead = &list.pHead; \
            (listRefPtr)->ppTail = &list.pTail; \
            (listRefPtr)->offset = list.offset; \
        } while(0)



#define LL2NodePtr_(type, pItem, listOffset)                        \
        ((type::LL2Node *)((unsigned char * )pItem + listOffset))

#define LL2NodePtr(type, list, pItem)           \
        LL2NodePtr_(type, pItem, list.offset)

#define LL2RefNodePtr(type, listRef, pItem)         \
        LL2NodePtr_(type, pItem, listRef.offset)


#define LL2IsItemLinked_(type, pItem, listOffset)                   \
        (LL2NodePtr_(type, pItem, listOffset)->pPrev != nullptr)

// NOTE - It's possible for an item to be linked, but not necessarily be a part of this list (i.e., there are multiple heads that all
//  use the same nodes as links and items can only be on one list). Querying this would require O(n) search.
#define LL2IsItemLinked(type, list, pItem)          \
        LL2IsItemLinked_(type, pItem, list.offset)

#define LL2RefIsItemLinked(type, listRef, pItem)        \
        LL2IsItemLinked_(type, pItem, listRef.offset)


    
#define LL2IsNodeLinked(node)                   \
    (node.pPrev != nullptr)


#define LL2AddHead_(type, ppListHead, ppListTail, listOffset, pItem)    \
    do {                                                                \
        if (LL2IsItemLinked_(type, pItem, listOffset))                  \
        {                                                               \
            LL_ASSERT(false);                                           \
            break;                                                      \
        }                                                               \
        auto * itemNode_ = LL2NodePtr_(type, pItem, listOffset);        \
        if (*ppListHead)                                                \
        {                                                               \
            LL2NodePtr_(type, *ppListHead, listOffset)->pPrev = pItem;  \
            itemNode_->pNext = *ppListHead;                             \
        }                                                               \
        else                                                            \
        {                                                               \
            itemNode_->pNext = (type *)LLEndOfList_;                    \
            *ppListTail = pItem;                                        \
        }                                                               \
        itemNode_->pPrev = (type *)LLEndOfList_;                        \
        *ppListHead = pItem;                                            \
    } while(0)

#define LL2AddHead(type, list, pItem)                                   \
        LL2AddHead_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL2RefAddHead(type, listRef, pItem)                             \
        LL2AddHead_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)

#define LL2Add(type, list, pItem)               \
        LL2AddHead(type, list, pItem)

#define LL2RefAdd(type, listRef, pItem)         \
        LL2RefAddHead(type, listRef, pItem)



// TODO - LL2IsItemLinked check will break if linked to a separate list that uses the same node?
//  What is the desired behavior in this case?
#define LL2AddTail_(type, ppListHead, ppListTail, listOffset, pItem)    \
        do {                                                            \
            if (LL2IsItemLinked_(type, pItem, listOffset))              \
            {                                                           \
                LL_ASSERT(false);                                       \
                break;                                                  \
            }                                                           \
            auto * itemNode_ = LL2NodePtr_(type, pItem, listOffset);    \
            if (*ppListTail)                                            \
            {                                                           \
                LL2NodePtr_(type, *ppListTail, listOffset)->pNext = pItem; \
                itemNode_->pPrev = *ppListTail;                         \
            }                                                           \
            else                                                        \
            {                                                           \
                itemNode_->pPrev = (type *)LLEndOfList_;                \
                *ppListHead = pItem;                                    \
            }                                                           \
            itemNode_->pNext = (type *)LLEndOfList_;                    \
            *ppListTail = pItem;                                        \
        } while(0)

            #define LL2AddTail(type, list, pItem)                               \
                LL2AddTail_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL2RefAddTail(type, listRef, pItem)                             \
            LL2AddTail_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)

    

#define LL2Remove_(type, ppListHead, ppListTail, listOffset, pItem)     \
            do {                                                        \
                auto * node_ = LL2NodePtr_(type, pItem, listOffset);    \
                if (!LL2IsItemLinked_(type, pItem, listOffset)) break;  \
                type * pPrev_ = node_->pPrev;                           \
                type * pNext_ = node_->pNext;                           \
                bool hasNext_ = pNext_ != (type *)LLEndOfList_;         \
                bool hasPrev_ = pPrev_ != (type *)LLEndOfList_;         \
                if (hasNext_ && hasPrev_)                               \
                {                                                       \
                    LL2NodePtr_(type, pNext_, listOffset)->pPrev = pPrev_; \
                    LL2NodePtr_(type, pPrev_, listOffset)->pNext = pNext_; \
                }                                                       \
                else if (hasNext_ && !hasPrev_)                         \
                {                                                       \
                    LL2NodePtr_(type, pNext_, listOffset)->pPrev = (type *)LLEndOfList_; \
                    *ppListHead = pNext_;                               \
                }                                                       \
                else if (!hasNext_ && hasPrev_)                         \
                {                                                       \
                    LL2NodePtr_(type, pPrev_, listOffset)->pNext = (type *)LLEndOfList_; \
                    *ppListTail = pPrev_;                               \
                }                                                       \
                else                                                    \
                {                                                       \
                    *ppListHead = nullptr;                              \
                    *ppListTail = nullptr;                              \
                }                                                       \
                node_->pPrev = nullptr;                                 \
                node_->pNext = nullptr;                                 \
            } while(0)

                #define LL2Remove(type, list, pItem)                                \
                    LL2Remove_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL2RefRemove(type, listRef, pItem)                              \
                LL2Remove_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)



#define LL2RemoveHead_(type, ppListHead, ppListTail, listOffset, pAssignTo) \
                do {                                                    \
                    type * headToRemove_ = *ppListHead;                 \
                    pAssignTo = headToRemove_;                          \
                    if (headToRemove_)                                  \
                    {                                                   \
                        auto * headToRemoveNode_ = LL2NodePtr_(type, headToRemove_, listOffset); \
                        if (headToRemoveNode_->pNext != (type *)LLEndOfList_) \
                        {                                               \
                            *ppListHead = headToRemoveNode_->pNext;     \
                            auto * newHeadNode_ = LL2NodePtr_(type, *ppListHead, listOffset); \
                            newHeadNode_->pPrev = (type *)LLEndOfList_; \
                        }                                               \
                        else                                            \
                        {                                               \
                            *ppListHead = nullptr;                      \
                            *ppListTail = nullptr;                      \
                        }                                               \
                        headToRemoveNode_->pNext = nullptr;             \
                        headToRemoveNode_->pPrev = nullptr;             \
                    }                                                   \
                } while (0)

                    #define LL2RemoveHead(type, list, pAssignTo)                            \
LL2RemoveHead_(type, &list.pHead, &list.pTail, list.offset, pAssignTo)

#define LL2RefRemoveHead(type, listRef, pAssignTo)                      \
    LL2RemoveHead_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pAssignTo)



#define LL2RemoveTail_(type, ppListHead, ppListTail, listOffset, pAssignTo) \
    do {                                                                \
        type * tailToRemove_ = *ppListTail;                             \
        pAssignTo = tailToRemove_;                                      \
        if (tailToRemove_)                                              \
        {                                                               \
            auto * tailToRemoveNode_ = LL2NodePtr_(type, tailToRemove_, listOffset); \
            if (tailToRemoveNode_->pPrev != (type *)LLEndOfList_)       \
            {                                                           \
                *ppListTail = tailToRemoveNode_->pPrev;                 \
                auto * newTailNode_ = LL2NodePtr_(type, *ppListTail, listOffset); \
                newTailNode_->pNext = (type *)LLEndOfList_;             \
            }                                                           \
            else                                                        \
            {                                                           \
                *ppListHead = nullptr;                                  \
                *ppListTail = nullptr;                                  \
            }                                                           \
            tailToRemoveNode_->pNext = nullptr;                         \
            tailToRemoveNode_->pPrev = nullptr;                         \
        }                                                               \
    } while (0)

#define LL2RemoveTail(type, list, pAssignTo)                            \
    LL2RemoveTail_(type, &list.pHead, &list.pTail, list.offset, pAssignTo)

#define LL2RefRemoveTail(type, listRef, pAssignTo)                      \
    LL2RemoveTail_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pAssignTo)



#define LL2Next_(type, pItem, listOffset)                               \
    ((LL2NodePtr_(type, pItem, listOffset)->pNext == (type *)LLEndOfList_) ? nullptr : LL2NodePtr_(type, pItem, listOffset)->pNext)

#define LL2Next(type, list, pItem)              \
    LL2Next_(type, pItem, list.offset)

#define LL2RefNext(type, listRef, pItem)        \
    LL2Next_(type, pItem, listRef.offset)



#define LL2Prev_(type, pItem, listOffset)                               \
    ((LL2NodePtr_(type, pItem, listOffset)->pPrev == (type *)LLEndOfList_) ? nullptr : LL2NodePtr_(type, pItem, listOffset)->pPrev)

#define LL2Prev(type, list, pItem)              \
    LL2Prev_(type, pItem, list.offset)

#define LL2RefPrev(type, listRef, pItem)        \
    LL2Prev_(type, pItem, listRef.offset)


    

#define LL2Clear_(type, ppListHead, ppListTail, listOffset)             \
    do {                                                                \
        while (*ppListHead)                                             \
        {                                                               \
            auto * pHeadNode_ = LL2NodePtr_(type, *ppListHead, listOffset); \
            auto * pHeadNext_ = LL2Next_(type, *ppListHead, listOffset); \
            pHeadNode_->pNext = nullptr;                                \
            pHeadNode_->pPrev = nullptr;                                \
            *ppListHead = pHeadNext_;                                   \
        }                                                               \
        *ppListTail = nullptr;                                          \
    } while(0)

#define LL2Clear(type, list)                                \
    LL2Clear_(type, &list.pHead, &list.pTail, list.offset)

#define LL2RefClear(type, listRef)                                  \
    LL2Clear_(type, listRef.ppHead, listRef.ppTail, listRef.offset)



#define LL2ClearWithoutUnlinking_(type, ppListHead, ppListTail)     \
    do { *ppListHead = nullptr; *ppListTail = nullptr; } while (0)

#define LL2ClearWithoutUnlinking(type, list)                    \
    LL2ClearWithoutUnlinking_(type, &list.pHead, &list.pTail)

#define LL2RefClearWithoutUnlinking(type, listRef)                  \
    LL2ClearWithoutUnlinking_(type, listRef.ppHead, listRef.ppTail)



#define LL2IsEmpty_(ppListHead)                 \
    (!(*ppListHead))

#define LL2IsEmpty(list)                        \
    LL2IsEmpty_(&list.pHead)

#define LL2RefIsEmpty(listRef)                  \
    LL2IsEmpty_(listRef.ppHead)

    

// NOTE - List 1 is cleared
#define LL2Combine_(type, ppList0Head, ppList0Tail, ppList1Head, ppList1Tail, listOffset) \
    do {                                                                \
        if (LL2IsEmpty_(ppList0Head))                                   \
        {                                                               \
            *ppList0Head = *ppList1Head;                                \
            *ppList0Tail = *ppList1Tail;                                \
        }                                                               \
        else if (!LL2IsEmpty_(ppList1Head))                             \
        {                                                               \
            auto * pNodeTail0_ = LL2NodePtr_(type, *ppList0Tail, listOffset); \
            auto * pNodeHead1_ = LL2NodePtr_(type, *ppList1Head, listOffset); \
            pNodeTail0_->pNext = *ppList1Head;                          \
            pNodeHead1_->pPrev = *ppList0Tail;                          \
            *ppList0Tail = *ppList1Tail;                                \
        }                                                               \
        *ppList1Head = nullptr;                                         \
        *ppList1Tail = nullptr;                                         \
    } while(0)

#define LL2Combine(type, combineParam)                                  \
    LL2Combine_(type, combineParam.ppHead0, combineParam.ppTail0, combineParam.ppHead1, combineParam.ppTail1, combineParam.offset)

        

#define ForLL2_(type, it, ppListHead, listOffset)                       \
    for (type * it = *ppListHead; it; it = LL2Next_(type, it, listOffset))
    
#define ForLL2(type, it, list)                  \
    ForLL2_(type, it, &list.pHead, list.offset)

#define ForLL2Ref(type, it, listRef)                    \
    ForLL2_(type, it, listRef.ppHead, listRef.offset)


// NOTE - This assumes that the newAddress has its intrusive pointers already set properly
#define LL2Relocate_(type, ppListHead, ppListTail, listOffset, prevAddress, newAddress) \
    do {                                                                \
        if (*ppListHead == prevAddress)                                 \
        {                                                               \
            *ppListHead = newAddress;                                   \
        }                                                               \
        if (*ppListTail == prevAddress)                                 \
        {                                                               \
            *ppListTail = newAddress;                                   \
        }                                                               \
        type * prev_ = LL2Prev_(type, newAddress, listOffset);          \
        if (prev_)                                                      \
        {                                                               \
            LL_ASSERT(LL2Next_(type, prev_, listOffset) == prevAddress); \
            auto * prevNode_ = LL2NodePtr_(type, prev_, listOffset);    \
            prevNode_->pNext = newAddress;                              \
        }                                                               \
        type * next_ = LL2Next_(type, newAddress, listOffset);          \
        if (next_)                                                      \
        {                                                               \
            LL_ASSERT(LL2Prev_(type, next_, listOffset) == prevAddress); \
            auto * nextNode_ = LL2NodePtr_(type, next_, listOffset);    \
            nextNode_->pPrev = newAddress;                              \
        }                                                               \
    } while (0)

#define LL2Relocate(type, list, prevAddress, newAddress)                \
    LL2Relocate_(type, &list.pHead, &list.pTail, list.offset, prevAddress, newAddress)

#define LL2RefRelocate(type, listRef, prevAddress, newAddress)          \
    LL2Relocate_(type, listRef.ppHead, listRef.ppTail, listRef.offset, prevAddress, newAddress)




//
// Author: Andrew Smith - alsmith.net
// License: MIT
//
#endif
