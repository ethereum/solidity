{
    let zero := 0
    tstore(zero, 5)
    let x := tload(zero)
    tstore(zero, 8)
    let y := tload(zero)
    tstore(zero, y)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let zero := 0
//         tstore(zero, 5)
//         let x := tload(zero)
//         tstore(zero, 8)
//         tstore(zero, tload(zero))
//     }
// }
