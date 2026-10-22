{
    let x := mload(0)
    if slt(x, 0) { x := sub(0, x) }
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := mload(0)
//     let condition := iszero(iszero(slt(x, 0)))
//     x := xor(x, and(sub(0, condition), xor(x, sub(0, x))))
// }
