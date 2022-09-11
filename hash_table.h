#pragma once

#include <cstdint>
#include <cstring>
#include <exception>
#include "allocator.h"
#include "common.h"
#include "hash.h"

#include <iostream>

template <uint8_t size_degree = 8>
struct HTAssistant
{
public:
    static constexpr uint8_t INITIAL_SIZE_DEGREE = size_degree;
    static_assert(INITIAL_SIZE_DEGREE >= 1, "size_degree < 1");

    inline size_t BufSize() const { return 1 << degree; }
    //! 0.5
    inline size_t MaxCount() const { return 1 << (degree - 1); }

    inline size_t Slot(size_t hash) const { return hash & Mask(); }

    inline size_t Mask() const { return BufSize() - 1; }

    inline bool NeedGrow(size_t x) const { return x > MaxCount(); }

    inline void Grow()
    {
        if (degree >= 48)
        {
            throw std::logic_error("Out of memory In HashTableGrower::Grow(...)");
        }
        degree += 2;
    }

    inline size_t Next(size_t x) const { return ((++x) & Mask()); }

private:
    uint8_t degree = INITIAL_SIZE_DEGREE;
};

template <typename Key>
struct HashTableCell
{
    using key_type = Key;
    using mapped_type = void;

    Key key;

    HashTableCell() { }

    HashTableCell(Key && key) : key(std::move(key)) { }

    const Key & GetKey() const { return key; }
    // mapped_type GetValue() const { return; }

    const Key & GetRawKey() const { return key; }

    hash_t GetHash() const { return Hash(key); }
    void SetHash(hash_t) { }

    bool IsOccupied() const { return key != Key{}; }
    void SetUnoccupied() { key = Key{}; }
};

//! https://en.wikipedia.org/wiki/Open_addressing
template <typename Cell, typename Allocator, typename Assisitant>
struct HashTable : private Allocator
{
public:
    HashTable() : tsize(0) { buf = reinterpret_cast<Cell *>(Allocator::Alloc(assisitant.BufSize() * sizeof(Cell))); }

    ~HashTable()
    {
        if (buf)
        {
            Allocator::Free(buf);
            buf = nullptr;
        }
    }

    size_t Size() const { return tsize; }

public:
    using Key = typename Cell::key_type;
    using key_type = Key;
    using mapped_type = typename Cell::mapped_type;
    using cell_type = Cell;
    using result_type = Cell *;

    result_type __attribute__((__always_inline__)) Lookup(const Key & key)
    {
        hash_t hash;
        size_t index = FindSlot(key, hash);
        if (!buf[index].IsOccupied())
        {
            return nullptr;
        }
        return &buf[index];
    }

    bool __attribute__((__always_inline__)) Emplace(Key key, result_type & cell)
    {
        hash_t hash;
        size_t index = FindSlot(key, hash);
        // already exist
        if (buf[index].IsOccupied())
        {
            cell = &buf[index];
            return false;
        }

        if (unlikely(assisitant.NeedGrow(tsize + 1)))
        {
            // std::cout << "key count : " << Size() << "\n";
            // std::cout << "resize : " << assisitant.BufSize() << "\n";
            // std::cout << "c : " << c << "\n";
            size_t old_buf_size = assisitant.BufSize();
            assisitant.Grow();
            Cell * new_buf = reinterpret_cast<Cell *>(Allocator::Alloc(assisitant.BufSize() * sizeof(Cell)));
            for (size_t i = 0; i < old_buf_size; ++i)
            {
                if (buf[i].IsOccupied())
                {
                    hash_t hash = buf[i].GetHash();
                    // hash_t hash = Hash(buf[i].GetRawKey());
                    size_t new_index = assisitant.Slot(hash);
                    __builtin_memcpy(static_cast<void *>(&new_buf[new_index]), static_cast<const void *>(&buf[i]), sizeof(Cell));
                }
            }
            Allocator::Free(buf);
            buf = new_buf;
            index = FindSlot(key, hash);
        }
        new (&buf[index]) Cell(std::move(key));
        buf[index].SetHash(hash);
        cell = &buf[index];
        tsize++;
        return true;
    }

    bool __attribute__((__always_inline__)) Erase(const Key & key)
    {
        hash_t hash;
        size_t index = FindSlot(key, hash);
        if (!buf[index].IsOccupied())
        {
            return false;
        }
        size_t next_index = index;
        while (true)
        {
            next_index = assisitant.Next(next_index);
            if (!buf[next_index].IsOccupied())
            {
                break;
            }
            size_t optimal_index = assisitant.Slot(buf[next_index].GetHash());
            // determine if k lies cyclically in (i,j]
            // |    i.k.j |
            // |....j i.k.| or  |.k..j i...|
            if (index <= next_index && (index < optimal_index && optimal_index <= next_index))
            {
                continue;
            }

            if (index > next_index && (index < optimal_index || optimal_index <= next_index))
            {
                continue;
            }

            __builtin_memcpy(static_cast<void *>(&buf[index]), static_cast<const void *>(&buf[next_index]), sizeof(Cell));
            index = next_index;
        }
        buf[index].SetUnoccupied();
        tsize--;

        return true;
    }

    size_t GetC() const { return c; }

private:
    // TODO: Split this function
    size_t __attribute__((__always_inline__)) FindSlot(const Key & key, hash_t & hash) const
    {
        hash = Hash(key);
        size_t index = assisitant.Slot(hash);
        // TODO(lokax): Need to process float/double
        // TODO: We can fast checkout hash value for string key.
        while (buf[index].IsOccupied() && (buf[index].GetHash() != hash || buf[index].GetRawKey() != key))
        {
            c++;
            // not found, move to next index
            index = assisitant.Next(index);
        }
        return index;
    }


protected:
    Assisitant assisitant;
    Cell * buf;
    size_t tsize;
    mutable size_t c = 0;
};