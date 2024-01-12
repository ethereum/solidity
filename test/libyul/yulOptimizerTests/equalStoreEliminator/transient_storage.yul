{
    tstore(0, 42)
    tstore(0, 42)
    let x := tload(0)
    tstore(0, 42)
    x := tload(0)
    tstore(0, 24)
    x := tload(0)
    tstore(0, 24)
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     tstore(0, 42)
//     tstore(0, 42)
//     let x := tload(0)
//     tstore(0, 42)
//     x := tload(0)
//     tstore(0, 24)
//     x := tload(0)
//     tstore(0, 24)
// }
