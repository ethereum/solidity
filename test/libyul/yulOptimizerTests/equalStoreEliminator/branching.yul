{
    let a := calldataload(0)
    let b := 20
    sstore(a, b)
    if calldataload(32) {
        sstore(a, b)
        pop(staticcall(0, 0, 0, 0, 0, 0))
        sstore(a, b)
    }
    sstore(a, b)
}
// ====
// EVMVersion: >=byzantium
// ----
// step: equalStoreEliminator
//
// {
//     let a := calldataload(0)
//     let b := 20
//     sstore(a, b)
//     if calldataload(32)
//     {
//         pop(staticcall(0, 0, 0, 0, 0, 0))
//     }
// }
