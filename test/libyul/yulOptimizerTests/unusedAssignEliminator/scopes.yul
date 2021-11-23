{
    let a
    {
        let b
        b := 2
        a := 2
    }
}
// ----
// step: unusedAssignEliminator
//
// {
//     let a
//     { let b }
// }
