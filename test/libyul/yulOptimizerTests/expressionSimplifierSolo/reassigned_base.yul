{
    let v := calldataload(0)
    let a := add(v, 2)
    v := calldataload(32)
    // `a` is not `v` plus 2 any more.
    sstore(sub(a, v), 1)
    let b := add(v, 2)
    sstore(sub(b, v), 1)
}
// ----
// step: expressionSimplifierSolo
//
// {
//     let v := calldataload(0)
//     let a := add(v, 2)
//     v := calldataload(32)
//     sstore(sub(a, v), 1)
//     let b := add(v, 2)
//     sstore(add(sub(v, v), 2), 1)
// }
