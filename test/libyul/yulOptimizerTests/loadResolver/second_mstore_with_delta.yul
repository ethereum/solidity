{
    let x := calldataload(1)
    let a := add(x, 10)
    let b := add(x, 42)
    mstore(a, 7)
    // does not invalidate the first store, because the
    // difference is larger than 32, even if the absolute
    // values are unknown
    mstore(b, 8)
    sstore(mload(a), mload(b))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         let a := add(x, 10)
//         let b := add(x, 42)
//         let _1 := 7
//         mstore(a, _1)
//         let _2 := 8
//         mstore(b, _2)
//         sstore(_1, _2)
//     }
// }
