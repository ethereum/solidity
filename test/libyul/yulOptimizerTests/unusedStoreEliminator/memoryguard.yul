{
    mstore(0x40, memoryguard(100))
    let free_mem_ptr := mload(0x40)
    // redundant
    mstore(free_mem_ptr, 100)
    // redundant
    mstore8(add(free_mem_ptr, 31), 200)
    mstore(free_mem_ptr, 300)
    return(free_mem_ptr, add(free_mem_ptr, 100))
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         mstore(0x40, memoryguard(100))
//         let free_mem_ptr := mload(0x40)
//         let _1 := 100
//         let _2 := 200
//         mstore8(add(free_mem_ptr, 31), _2)
//         mstore(free_mem_ptr, 300)
//         return(free_mem_ptr, add(free_mem_ptr, 100))
//     }
// }
