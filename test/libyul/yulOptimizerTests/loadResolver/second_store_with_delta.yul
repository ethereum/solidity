{
    let x := calldataload(1)
    let a := add(x, 10)
    let b := add(x, 20)
    sstore(a, 7)
    // does not invalidate the first store, because the
    // difference is a constant, even if the absolute
    // values are unknown
    sstore(b, 8)
    mstore(sload(a), sload(b))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         let a := add(x, 10)
//         let b := add(x, 20)
//         let _1 := 7
//         sstore(a, _1)
//         let _2 := 8
//         sstore(b, _2)
//         mstore(_1, _2)
//     }
// }
