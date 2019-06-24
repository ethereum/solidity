{
    let a := calldataload(0)
    sstore(a, 6)
    a := calldataload(2)
    mstore(0, sload(a))
}
// ====
// step: loadResolver
// ----
// {
//     let _1 := 0
//     let a := calldataload(_1)
//     sstore(a, 6)
//     a := calldataload(2)
//     mstore(_1, sload(a))
// }
