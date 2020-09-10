#ifndef ALS_LL2_H
#define ALS_LL2_H

//
// Macros for creating and working with intrusive doubly linked lists
//
// Requires:
//  -offsetof macro
//  -uintptr_t
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
//

#define DefineLL2(type)                     \
    struct LL2Node                          \
    {                                       \
        type * pPrev;                       \
        type * pNext;                       \
    };                                      \
    struct LL2Ref                           \
    {                                       \
        type ** ppHead;                     \
        type ** ppTail;                     \
        uintptr_t offset;                   \
    };                                      \
    struct LL2CombineParam                  \
    {                                       \
        type ** ppHead0;                    \
        type ** ppTail0;                    \
        type ** ppHead1;                    \
        type ** ppTail1;                    \
        uintptr_t offset;                   \
    }
    

        
#define DeclareLL2(type, linkMember)                                    \
        struct LL2_##type##_##linkMember                                \
        {                                                               \
            type * pHead;                                               \
            type * pTail;                                               \
            static const uintptr_t offset = offsetof(type, linkMember); \
        };                                                              \
        LL2_##type##_##linkMember



// NOTE - DeclareLL2 makes a unique type for each list, and stores the offset as a compile time constant.
//  If you want to operate on a list that you select at runtime, you can do so by making an LL2Ref
#define LL2MakeRef(listRefPtr, list)                    \
    do {                                                \
        (listRefPtr)->ppHead = &list.pHead;             \
        (listRefPtr)->ppTail = &list.pTail;             \
        (listRefPtr)->offset = list.offset;             \
    } while(0)



// NOTE - Null indicates an empty list. For non-empty lists, the end is indicated by a
//  sentinel value. This lets items trivially check if they are members of a list by
//  checking for null. (The sentinel value is required to make this work if they are
//  the only member of the list!)
#define LL2EndOfList_ 0x1



#define LL2NodePtr_(type, pItem, listOffset)                  \
    ((type::LL2Node *)((unsigned char * )pItem + listOffset))

#define LL2NodePtr(type, list, pItem)                 \
    LL2NodePtr_(type, pItem, list.offset)

#define LL2RefNodePtr(type, listRef, pItem)   \
    LL2NodePtr_(type, pItem, listRef.offset)


#define LL2IsItemLinked_(type, pItem, listOffset)             \
    (LL2NodePtr_(type, pItem, listOffset)->pPrev != nullptr)

// NOTE - It's possible for an item to be linked, but not necessarily be a part of this list (i.e., there are multiple heads that all
//  use the same nodes as links and items can only be on one list). Querying this would require O(n) search.
#define LL2IsItemLinked(type, list, pItem)      \
    LL2IsItemLinked_(type, pItem, list.offset)

#define LL2RefIsItemLinked(type, listRef, pItem)   \
    LL2IsItemLinked_(type, pItem, listRef.offset)


    
#define LL2IsNodeLinked(node) \
    (node.pPrev != nullptr)


// TODO - LL2IsItemLinked check ight break if linked to a separate list that uses the same node?
//  What is the desired behavior in this case?
#define LL2Add_(type, ppListHead, ppListTail, listOffset, pItem)        \
    do {                                                                \
        if (LL2IsItemLinked_(type, pItem, listOffset)) break;           \
        auto * itemNode_ = LL2NodePtr_(type, pItem, listOffset);        \
        if (*ppListHead)                                                \
        {                                                               \
            LL2NodePtr_(type, *ppListHead, listOffset)->pPrev = pItem;  \
            itemNode_->pNext = *ppListHead;                             \
        }                                                               \
        else                                                            \
        {                                                               \
            itemNode_->pNext = (type *)LL2EndOfList_;                   \
            *ppListTail = pItem;                                        \
        }                                                               \
        itemNode_->pPrev = (type *)LL2EndOfList_;                       \
        *ppListHead = pItem;                                            \
    } while(0)

#define LL2Add(type, list, pItem) \
    LL2Add_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL2RefAdd(type, listRef, pItem) \
    LL2Add_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)

    

#define LL2Remove_(type, ppListHead, ppListTail, listOffset, pItem) \
    do {                                                                \
        auto * node_ = LL2NodePtr_(type, pItem, listOffset);            \
        type * pPrev_ = node_->pPrev;                                   \
        type * pNext_ = node_->pNext;                                   \
        if (!LL2IsItemLinked_(type, pItem, listOffset)) break;          \
        bool hasNext_ = pNext_ != (type *)LL2EndOfList_;                \
        bool hasPrev_ = pPrev_ != (type *)LL2EndOfList_;                \
        if (hasNext_ && hasPrev_)                                       \
        {                                                               \
            LL2NodePtr_(type, pNext_, listOffset)->pPrev = pPrev_;      \
            LL2NodePtr_(type, pPrev_, listOffset)->pNext = pNext_;      \
        }                                                               \
        else if (hasNext_ && !hasPrev_)                                 \
        {                                                               \
            LL2NodePtr_(type, pNext_, listOffset)->pPrev = (type *)LL2EndOfList_; \
            *ppListHead = pNext_;                                       \
        }                                                               \
        else if (!hasNext_ && hasPrev_)                                 \
        {                                                               \
            LL2NodePtr_(type, pPrev_, listOffset)->pNext = (type *)LL2EndOfList_; \
            *ppListTail = pPrev_;                                       \
        }                                                               \
        else                                                            \
        {                                                               \
            *ppListHead = nullptr;                                      \
            *ppListTail = nullptr;                                      \
        }                                                               \
        node_->pPrev = nullptr;                                         \
        node_->pNext = nullptr;                                         \
    } while(0)

#define LL2Remove(type, list, pItem)                                \
    LL2Remove_(type, &list.pHead, &list.pTail, list.offset, pItem)

#define LL2RefRemove(type, listRef, pItem)                              \
    LL2Remove_(type, listRef.ppHead, listRef.ppTail, listRef.offset, pItem)



#define LL2RemoveHead_(type, ppListHead, ppListTail, listOffset)        \
          do {                                                          \
              type * headToRemove_ = *ppListHead;                       \
              if (headToRemove_)                                        \
              {                                                         \
                  auto * headToRemoveNode_ = LL2NodePtr_(type, headToRemove_, listOffset); \
                  if (headToRemoveNode_->pNext != (type *)LL2EndOfList_) \
                  {                                                     \
                      *ppListHead = headNode->pNext_;                   \
                  }                                                     \
                  else                                                  \
                  {                                                     \
                      *ppListHead = nullptr;                            \
                      *ppListTail = nullptr;                            \
                  }                                                     \
                  headToRemoveNode_->pNext = nullptr;                   \
                  headToRemoveNode_->pPrev = nullptr;                   \
              }                                                         \
          } while (0)

#define LL2RemoveHead(type, list)                               \
    LL2RemoveHead_(type, &list.pHead, &list.pTail, list.offset)

#define LL2RefRemoveHead(type, listRef)                     \
    LL2RemoveHead_(type, listRef.ppHead, listRef.ppTail, listRef.offset)



#define LL2Next_(type, pItem, listOffset)                             \
    ((LL2NodePtr_(type, pItem, listOffset)->pNext == (type *)LL2EndOfList_) ? nullptr : LL2NodePtr_(type, pItem, listOffset)->pNext)

#define LL2Next(type, list, pItem)            \
    LL2Next_(type, pItem, list.offset)

#define LL2RefNext(type, listRef, pItem)      \
    LL2Next_(type, pItem, listRef.offset)



#define LL2Prev_(type, pItem, listOffset)                               \
        ((LL2NodePtr_(type, pItem, listOffset)->pPrev == (type *)LL2EndOfList_) ? nullptr : LL2NodePtr_(type, pItem, listOffset)->pPrev)

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

        

#define ForLL2_(type, it, ppListHead, listOffset)                      \
    for (type * it = *ppListHead; it; it = LL2Next_(type, it, listOffset))
    
#define ForLL2(type, it, list)                  \
    ForLL2_(type, it, &list.pHead, list.offset)

#define ForLL2Ref(type, it, listRef)                    \
    ForLL2_(type, it, listRef.ppHead, listRef.offset)



//
// Author: Andrew Smith - alsmith.net
// License: MIT
//
#endif
