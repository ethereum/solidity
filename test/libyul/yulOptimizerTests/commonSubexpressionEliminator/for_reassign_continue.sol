{
    let x := 0
    for {} sload(0) { let t := 0 /* cannot replace this because of continue */ } {
        x := 0
        if calldataload(0) { x := 2 continue }
        let r := 0
        if calldataload(1) { x := 2 revert(0, 0) }
        let s := 0
    }
    // cannot replace this
    let u := 0
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
//         let r := x
//         if calldataload(1)
//         {
//             x := 2
//             revert(0, 0)
//         }
//         let s := x
//     }
//     let u := 0
// }
