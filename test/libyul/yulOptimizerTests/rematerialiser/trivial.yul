{
    let a := 1
    let b := a
    mstore(0, b)
}
// ====
// step: rematerialiser
// ----
// {
//     let a := 1
//     let b := 1
//     mstore(0, 1)
// }
