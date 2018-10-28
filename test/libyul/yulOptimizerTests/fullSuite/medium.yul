{
    function allocate(size) -> p {
        p := mload(0x40)
        mstore(0x40, add(p, size))
    }
    function array_index_access(array, index) -> p {
        p := add(array, mul(index, 0x20))
    }
    pop(allocate(0x20))
    let x := allocate(0x40)
    mstore(array_index_access(x, 3), 2)
}
// ----
// fullSuite
// {
//     mstore(0x40, add(mload(0x40), 0x20))
//     let allocate_p_2_1 := mload(0x40)
//     mstore(0x40, add(allocate_p_2_1, 0x40))
//     mstore(add(allocate_p_2_1, 96), 2)
// }
