#ifndef __PAGE_MAP_HPP__
#define __PAGE_MAP_HPP__ 1

#include"Common.hh"
#include"ObjectPool.hpp"

// 32位，寻址2^32，4KB一页，页号由20位表示【】
// 64位，寻址2^64，4KB一页，页号由52位表示

/// @brief 基数树
/// @tparam BITS 
template<size_t BITS>
struct PageMap2
{
public:
    PageMap2();
public:
    void Set(size_t page_id, void* value);
    void Del(size_t page_id);
    void* Get(size_t page_id);
private:
    static const size_t ROOT_BITS = BITS / 2;
    static const size_t LEAF_BITS = BITS - ROOT_BITS;
    static const size_t ROOT_LENGTH = 1<<ROOT_BITS;
    static const size_t LEAF_LENGTH = 1<<LEAF_BITS;

    struct _leaf_node
    {
        void* _node[LEAF_LENGTH];
    };

    static ObjectPool<_leaf_node> _objPool;
    _leaf_node* _root[ROOT_LENGTH];
};

template<size_t BITS>
struct PageMap3
{
public:
    PageMap3();
    // ~PageMap3();
public:
    void Set(size_t page_id, void* value);
    void Del(size_t page_id);
    void* Get(size_t page_id);
private:
    static const size_t ROOT_BITS = (BITS + 2) / 3;
    static const size_t LEAF_BITS = BITS - ROOT_BITS * 2;
    static const size_t ROOT_LENGTH = 1<<ROOT_BITS;
    static const size_t LEAF_LENGTH = 1<<LEAF_BITS;

    struct _leaf_node
    {
        void* _node[LEAF_LENGTH];
        // size_t num = 0;
    };

    struct _interior_node
    {
        _leaf_node* _node[ROOT_LENGTH];
    };

    static ObjectPool<_leaf_node, 512*1024> _leafPool;
    static ObjectPool<_interior_node, 2*1024*1024> _interPool;

    _interior_node* _root[ROOT_LENGTH];
};



////////////////////////////////////////////////////////////////////////  PageMap_2level
template<size_t BITS>
ObjectPool<typename PageMap2<BITS>::_leaf_node> PageMap2<BITS>::_objPool;

template <size_t BITS>
PageMap2<BITS>::PageMap2()
{
    memset(_root, 0, sizeof(_root));
}

template <size_t BITS>
void PageMap2<BITS>::Set(size_t page_id, void *value)
{
    size_t _root_id = page_id>>LEAF_BITS;
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);
    if(_root[_root_id] == nullptr){
        _root[_root_id] = _objPool.New();
    }
    _root[_root_id]->_node[_leaf_id] = value;
}

template <size_t BITS>
void PageMap2<BITS>::Del(size_t page_id)
{
    size_t _root_id = page_id>>LEAF_BITS;
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);
    assert(_root[_root_id]);
    assert(_root[_root_id]->_node[_leaf_id]);
    _root[_root_id]->_node[_leaf_id] = nullptr;
}

template <size_t BITS>
void *PageMap2<BITS>::Get(size_t page_id)
{
    size_t _root_id = page_id>>LEAF_BITS;
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);
    if(_root[_root_id] && _root[_root_id]->_node[_leaf_id]){
        return _root[_root_id]->_node[_leaf_id];
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////  PageMap_3level

template<size_t BITS>
ObjectPool<typename PageMap3<BITS>::_interior_node, 2*1024*1024> PageMap3<BITS>::_interPool;

template<size_t BITS>
ObjectPool<typename PageMap3<BITS>::_leaf_node, 512*1024> PageMap3<BITS>::_leafPool;

template <size_t BITS>
PageMap3<BITS>::PageMap3()
{
    memset(_root, 0, sizeof(_root));
}

// template <size_t BITS>
// PageMap3<BITS>::~PageMap3()
// {
//     for(int i = 0; i < ROOT_LENGTH; ++i){
//         auto interPtr = _root[i];
//         if(interPtr != nullptr){
//             for(int j = 0; j < ROOT_LENGTH; ++j){
//                 auto leafPtr = _root[i]->_node[j];
//                 if(leafPtr != nullptr){
//                     _leafPool.Delete(leafPtr);
//                 }
//             }
//             _interPool.Delete(interPtr);
//         }
//     }
// }

template <size_t BITS>
void PageMap3<BITS>::Set(size_t page_id, void *value)
{
    assert(value);

    size_t _root_id = page_id>>(LEAF_BITS + ROOT_BITS);
    size_t _inter_id = (page_id>>LEAF_BITS) & (ROOT_LENGTH - 1);
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);

    if(_root[_root_id] == nullptr){
        _root[_root_id] = _interPool.New();
    }
    if(_root[_root_id]->_node[_inter_id] == nullptr){
        _root[_root_id]->_node[_inter_id] = _leafPool.New();
    }
    if(_root[_root_id]->_node[_inter_id]->_node[_leaf_id] == nullptr){
        // _root[_root_id]->_node[_inter_id]->num++;
    }
    _root[_root_id]->_node[_inter_id]->_node[_leaf_id] = value;
}

template <size_t BITS>
void PageMap3<BITS>::Del(size_t page_id)
{
    
    size_t _root_id = page_id>>(LEAF_BITS + ROOT_BITS);
    size_t _inter_id = (page_id>>LEAF_BITS) & (ROOT_LENGTH - 1);
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);

    assert(_root[_root_id]);
    assert(_root[_root_id]->_node[_inter_id]);
    assert(_root[_root_id]->_node[_inter_id]->_node[_leaf_id]);

    _root[_root_id]->_node[_inter_id]->_node[_leaf_id] = nullptr;
    // _root[_root_id]->_node[_inter_id]->num--;

    // if(_root[_root_id]->_node[_inter_id]->num == 0){
    //     _leafPool.Delete(_root[_root_id]->_node[_inter_id]);
    //     _root[_root_id]->_node[_inter_id] = nullptr;
    // }
    // if(inode->num == 0){
    //     _interPool.Delete(inode);
    //     _root[_root_id] = nullptr;
    // }
}

template <size_t BITS>
void *PageMap3<BITS>::Get(size_t page_id)
{
    size_t _root_id = page_id>>(LEAF_BITS + ROOT_BITS);
    size_t _inter_id = (page_id>>LEAF_BITS) & (ROOT_LENGTH - 1);
    size_t _leaf_id = page_id & (LEAF_LENGTH - 1);

    if(_root[_root_id] && _root[_root_id]->_node[_inter_id] && _root[_root_id]->_node[_inter_id]->_node[_leaf_id]){
        return _root[_root_id]->_node[_inter_id]->_node[_leaf_id];
    }
    return nullptr;
}

#endif