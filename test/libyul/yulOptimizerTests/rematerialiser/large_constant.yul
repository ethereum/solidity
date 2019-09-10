{
    // Constants cost depending on their magnitude.
    // Do not rematerialize large constants.
    let a := 0xffffffffffffffffffffff
    mstore(a, a)
}
// ====
// step: rematerialiser
// ----
// {
//     let a := 0xffffffffffffffffffffff
//     mstore(a, a)
// }
