{
    let x := 0
    for {} sload(0) { let t := 0 } {
        x := 0
        if calldataload(0) { x := 2 continue }
    }
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let x := 0
//     for { } sload(0) { let t := 0 }
//     {
//         x := 0
//         if calldataload(x)
//         {
//             x := 2
//             continue
//         }
//     }
// }
