{
    tstore(0, 5)
    let x:= tload(0)
    tstore(0, 8)
    let y := tload(0)
    tstore(0, y)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedPruner
//
// {
//     {
//         tstore(0, 5)
//         tstore(0, 8)
//         let y := tload(0)
//         tstore(0, y)
//     }
// }
