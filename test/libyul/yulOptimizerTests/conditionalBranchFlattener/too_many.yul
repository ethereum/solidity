{
    let x := mload(0)
    let y := mload(0)			
    if slt(x, 0) {
        x := sub(0, x)
        y := add(y, 2)
    }
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := mload(0)
//     let y := mload(0)
//     if slt(x, 0)
//     {
//         x := sub(0, x)
//         y := add(y, 2)
//     }
// }
