{
    for {
        let a := 2
        // Should not be removed, even though you might think
        // it goes out of scope
        a := 3
    } a { a := add(a, 1) }
    {
        a := 7
    }
}
// ----
// redundantAssignEliminator
// {
//     for {
//         let a := 2
//         a := 3
//     }
//     a
//     {
//         a := add(a, 1)
//     }
//     {
//         a := 7
//     }
// }
