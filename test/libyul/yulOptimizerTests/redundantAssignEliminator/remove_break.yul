{
    let i := 0
    for {} lt(i, 2) { i := add(i, 1) }
    {
        let x
        x := 1337
        if lt(i,1) {
            x := 42
            break
        }
        mstore(0, x)
    }
}
// ----
// step: redundantAssignEliminator
//
// {
//     let i := 0
//     for { } lt(i, 2) { i := add(i, 1) }
//     {
//         let x
//         x := 1337
//         if lt(i, 1) { break }
//         mstore(0, x)
//     }
// }
