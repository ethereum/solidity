{
    let b := 20
    for {
        let a := 20
    }
    lt(a, 40)
    {
        a := add(a, 2)
    }
    {
        a := a
        mstore(0, a)
        a := add(a, 10)
    }
    stop()
}

// ====
// step: deadCodeEliminator
// ----
// {
//     let b := 20
//     let a := 20
//     for { } lt(a, 40) { a := add(a, 2) }
//     {
//         a := a
//         mstore(0, a)
//         a := add(a, 10)
//     }
//     stop()
// }
