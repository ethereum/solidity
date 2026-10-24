{
    let a := calldataload(0)
    let b := 20
    sstore(a, b)
    if calldataload(32) {
        sstore(a, b)
        pop(sload(a))
        verbatim_0i_0o("xyz")
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
//     if calldataload(32)
//     {
//         pop(sload(a))
//         verbatim_0i_0o("xyz")
//     }
//     sstore(a, b)
// }
