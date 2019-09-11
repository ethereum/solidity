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
//     {
//         let p := 0
//         let p_1 := p
//         let p_2 := p
//         let p_3 := p
//         let p_4 := p
//         let p_5 := p
//         let p_6 := p
//         let p_7 := p
//         let p_8 := p
//         let p_9 := p
//         let p_10 := p
//         let p_11 := mload(0x40)
//         mstore(0x40, add(p_11, 0x20))
//         mstore(0x40, add(p_11, 96))
//         mstore(add(p_11, 128), 2)
//         let _1 := 1
//         mstore(0x40, 0x20)
//         for { } iszero(p) { }
//         {
//             p := 0
//             if p_1 { break }
//             p_1 := p
//             if p_2 { break }
//             p_2 := p
//             if p_3 { break }
//             p_3 := p
//             if p_4 { break }
//             p_4 := p
//             if p_5 { break }
//             p_5 := p
//             if p_6 { break }
//             p_6 := p
//             if p_7 { break }
//             p_7 := p
//             if p_8 { break }
//             p_8 := p
//             if p_9 { break }
//             p_9 := p
//             if p_10 { break }
//             p_10 := p
//             if iszero(_1) { break }
//             if _1 { break }
//             _1 := p
//             mstore(128, 0x40)
//         }
//     }
// }
