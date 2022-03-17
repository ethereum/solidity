{
    let x := 0
    let y := 2
    for {} sload(0) { let t := add(y, 0) } {
        x := 0
        if calldataload(0) { x := 2 revert(0, 0) }
        if calldataload(0) { x := 2 break }
        if calldataload(0) { x := 2 continue }
        // zero here can be replaced
		let r := sload(0)
    }
    // not anymore because of break/continue
    let u := 0
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let x := 0
//     let y := 2
//     for { } sload(0) { let t := add(y, 0) }
//     {
//         x := 0
//         if calldataload(x)
//         {
//             x := y
//             revert(0, 0)
//         }
//         if calldataload(x)
//         {
//             x := y
//             break
//         }
//         if calldataload(x)
//         {
//             x := y
//             continue
//         }
//         let r := sload(x)
//     }
//     let u := 0
// }
