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
//         for { } and(y, 8) { if y { revert(0, 0) } }
//         {
//             if y { continue }
//             sstore(1, 0)
//         }
//         if y { revert(0, 0) }
//     }
// }
