{
    let a := 1
    let b := mload(a)
    let c := a
    mstore(add(a, b), c)
}
// ====
// step: rematerialiser
// ----
// {
//     let a := 1
//     let b := mload(1)
//     let c := 1
//     mstore(add(1, b), 1)
// }
