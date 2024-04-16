{
    let x
    {
        let y := msize()
        x := add(4, y)
    }
    x := keccak256(0, mod(x, 2))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x
//         let y := msize()
//         let _1 := 4
//         x := add(_1, y)
//         x := keccak256(0, addmod(_1, y, 2))
//     }
// }
