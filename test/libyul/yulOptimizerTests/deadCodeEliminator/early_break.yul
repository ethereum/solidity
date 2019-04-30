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
        break
        mstore(0, a)
        a := add(a, 10)
    }
}

// ====
// step: deadCodeEliminator
// ----
// {
//     let a := 20
//     for {
//     }
//     lt(a, 40)
//     {
//         a := add(a, 2)
//     }
//     {
//         a := a
//         break
//     }
// }
