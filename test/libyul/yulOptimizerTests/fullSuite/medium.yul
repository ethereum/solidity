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
    if 0 {
        mstore(0x40, 0x20)
    }
    mstore(0x40, 0x20)
    sstore(0, array_index_access(x, 3))
    sstore(1, mload(0x40))
}
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, add(mload(0x40), 128))
//         sstore(1, 0x20)
//     }
// }
