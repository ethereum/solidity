{
    let v := calldataload(0)
    let a := v
    v := calldataload(32)
    // `a` still holds the old value of `v`.
    sstore(a, v)
    let b := a
    sstore(b, 1)
}
// ----
// step: rematerialiserSolo
//
// {
//     let v := calldataload(0)
//     let a := v
//     v := calldataload(32)
//     sstore(a, v)
//     let b := a
//     sstore(a, 1)
// }
