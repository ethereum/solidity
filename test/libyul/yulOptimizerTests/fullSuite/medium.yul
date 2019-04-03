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
// ====
// step: fullSuite
// ----
// {
//     let _1 := 0x40
//     mstore(_1, add(mload(_1), 0x20))
//     let p := mload(_1)
//     mstore(_1, add(p, _1))
//     mstore(add(p, 96), 2)
//     mstore(_1, 0x20)
// }
