{
    let a := 2
    a := 7
    let b := 8
    b := a
    a := b
}
// ----
// redundantAssignEliminator
// {
//     let a := 2
//     a := 7
//     let b := 8
//     b := a
// }
