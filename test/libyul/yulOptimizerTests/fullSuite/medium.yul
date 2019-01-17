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
    if sub(2,1) {
        for { switch mul(1,2) case 2 { mstore(0x40, 0x20) } } sub(1,1) {} { mstore(0x80, 0x40) }
    }
}
// ----
// fullSuite
// {
//     let allocate__19 := 0x40
//     mstore(allocate__19, add(mload(allocate__19), 0x20))
//     let allocate_p_35_39 := mload(allocate__19)
//     mstore(allocate__19, add(allocate_p_35_39, allocate__19))
//     mstore(add(allocate_p_35_39, 96), 2)
//     mstore(allocate__19, 0x20)
// }
