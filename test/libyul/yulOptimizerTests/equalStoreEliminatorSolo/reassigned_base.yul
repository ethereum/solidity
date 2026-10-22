{
    let v := calldataload(0)
    let x := calldataload(64)
    let a := add(v, 2)
    sstore(a, x)
    sstore(a, x)
    v := calldataload(32)
    let c := add(v, 2)
    sstore(c, 2)
    // `c` may be `a`, so this store is needed.
    sstore(a, x)
}
// ----
// step: equalStoreEliminatorSolo
//
// {
//     let v := calldataload(0)
//     let x := calldataload(64)
//     let a := add(v, 2)
//     sstore(a, x)
//     v := calldataload(32)
//     let c := add(v, 2)
//     sstore(c, 2)
//     sstore(a, x)
// }
