{
    let a
    {
        let b
        b := 2
        a := 2
    }
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let a
//     {
//         let b
//     }
// }
