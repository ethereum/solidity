{
    let y := mload(0x20)
    for {} and(y, 8) { if y { revert(0, 0) } } {
        if y { continue }
        sstore(1, y)
    }
    if y { revert(0, 0) }
}
// ----
// step: fullSuite
//
// {
//     {
//         let y := mload(0x20)
//         let _1 := iszero(and(y, 8))
//         for { }
//         iszero(_1)
//         {
//             if y
//             {
//                 let _2 := 0
//                 revert(_2, _2)
//             }
//         }
//         {
//             if y { continue }
//             sstore(1, 0)
//         }
//         if y { revert(0, 0) }
//     }
// }
