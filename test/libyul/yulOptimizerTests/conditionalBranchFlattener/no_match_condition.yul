{
    let x := mload(0)
    if slt(0, x) { x := sub(0, x) }
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := mload(0)
//     let condition := iszero(iszero(slt(0, x)))
//     x := xor(x, and(sub(0, condition), xor(x, sub(0, x))))
// }
