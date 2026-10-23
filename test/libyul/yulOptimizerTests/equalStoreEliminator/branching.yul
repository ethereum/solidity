{
    let a := calldataload(0)
    let b := 20
    sstore(a, b)
    if calldataload(32) {
        sstore(a, b)
        pop(sload(a))
        sstore(a, b)
    }
    sstore(a, b)
}
// ----
// step: equalStoreEliminator
//
// {
//     let a := calldataload(0)
//     let b := 20
//     sstore(a, b)
//     if calldataload(32) { pop(sload(a)) }
// }
