{
    let a
    a := 0
    a := mload(0)
}
// ----
// step: unusedAssignEliminator
//
// {
//     let a
//     a := mload(0)
// }
