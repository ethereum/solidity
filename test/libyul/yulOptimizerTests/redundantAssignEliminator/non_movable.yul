{
    let a
    a := 0
    a := mload(0)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let a
//     a := mload(0)
// }
