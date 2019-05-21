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
// ====
// step: sloadResolver
// ----
// {
//     let x := calldataload(1)
//     let a := add(x, 10)
//     let b := add(x, 20)
//     let _4 := 7
//     sstore(a, _4)
//     let _5 := 8
//     sstore(b, _5)
//     mstore(_4, _5)
// }
