{
    let x := 0
    for {} sload(0) { let t := 0 /* can replace this */ } {
        x := 0
        if calldataload(0) { x := 2 break }
        let r := 0
        if calldataload(1) { x := 3 revert(0, 0) }
        let s := 0
        if calldataload(0) { continue }
        let v := 0
        x := 4
        revert(0, 0)
    }
    // cannot replace this
    let u := 0
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let x := 0
//     for { } sload(0) { let t := x }
//     {
//         x := 0
//         if calldataload(x)
//         {
//             x := 2
//             break
//         }
//         let r := x
//         if calldataload(1)
//         {
//             x := 3
//             revert(0, 0)
//         }
//         let s := x
//         if calldataload(x) { continue }
//         let v := x
//         x := 4
//         revert(0, 0)
//     }
//     let u := 0
// }
