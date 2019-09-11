{
    let y := mload(0x20)
    for {} and(y, 8) { if y { revert(0, 0) } } {
        if y { continue }
        sstore(1, y)
    }
    if y { revert(0, 0) }
}
// ====
// step: fullSuite
// ----
// {
//     {
//         let y := mload(0x20)
//         for { }
//         and(y, 8)
//         {
//             if y { revert(0, 0) }
//             y := 0
//         }
//         {
//             if y { continue }
//             y := 0
//             sstore(1, y)
//         }
//         if y { revert(0, 0) }
//     }
// }
