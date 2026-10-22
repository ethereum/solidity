{
    let v := calldataload(0)
    let a := add(v, 2)
    let b := add(v, 2)
    v := calldataload(32)
    // `v` changed, so this is not `a`.
    let c := add(v, 2)
    sstore(a, b)
    sstore(c, 1)
}
// ----
// step: commonSubexpressionEliminatorSolo
//
// {
//     let v := calldataload(0)
//     let a := add(v, 2)
//     let b := a
//     v := calldataload(32)
//     let c := add(v, 2)
//     sstore(a, a)
//     sstore(c, 1)
// }
