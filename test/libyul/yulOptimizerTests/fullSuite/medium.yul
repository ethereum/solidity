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
//     {
//         let _18 := 0x20
//         let allocate__7 := 0x40
//         mstore(allocate__7, add(mload(allocate__7), _18))
//         let allocate_p_12_31 := mload(allocate__7)
//         mstore(allocate__7, add(allocate_p_12_31, allocate__7))
//         mstore(add(allocate_p_12_31, 96), 2)
//     }
// }
