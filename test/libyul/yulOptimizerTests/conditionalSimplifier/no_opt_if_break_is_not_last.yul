{
    let x := mload(0)
    for {} 1 {} {
        if x { sstore(7, 8) break sstore(8, 9) }
        sstore(1, x)
        if x { sstore(7, 8) break }
        sstore(10, x)
    }
}
// ====
// step: conditionalSimplifier
// ----
// {
//     let x := mload(0)
//     for { } 1 { }
//     {
//         if x
//         {
//             sstore(7, 8)
//             break
//             sstore(8, 9)
//         }
//         sstore(1, x)
//         if x
//         {
//             sstore(7, 8)
//             break
//         }
//         x := 0
//         sstore(10, x)
//     }
// }
