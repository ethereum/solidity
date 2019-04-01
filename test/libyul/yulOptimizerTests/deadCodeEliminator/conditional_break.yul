{
    for {
        let a := 20
    }
    lt(a, 40)
    {
        a := add(a, 2)
    }
    {
        a := a
        if lt(a, 0)
        { break }
    }
}

// ----
// deadCodeEliminator
// {
//     for {
//         let a := 20
//     }
//     lt(a, 40)
//     {
//         a := add(a, 2)
//     }
//     {
//         a := a
//         if lt(a, 0)
//         {
//             break
//         }
//     }
// }
