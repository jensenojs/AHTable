
#include <iostream>
#include <unordered_map>
#include "allocator.h"
#include "hash_table.h"

int main()
{
    std::unordered_map<float, int> hash_table;
    // std::unordered_map<int, int>::key_type;
    using HashSet = HashTable<HashTableCell<int>, DefaultAllocator, HashTableGrower<>>;
    HashSet my_hash_table;
    HashSet::cell_type * cell = nullptr;
    bool inserted = my_hash_table.Emplace(1, cell);
    if (inserted)
    {
        std::cout << cell->GetKey() << "\n";
    }
    my_hash_table.Emplace(2, cell);
    my_hash_table.Emplace(3, cell);
    my_hash_table.Emplace(4, cell);

    cell = my_hash_table.Lookup(3);
    std::cout << "3 = 3 ? : " << (cell->GetKey() == 3) << "\n";

    bool erased = my_hash_table.Erase(3);
    std::cout << "erase ? : " << erased << "\n";

    cell = my_hash_table.Lookup(3);
    std::cout << "cell is null? ? : " << (cell == nullptr) << "\n";
    return 0;
}