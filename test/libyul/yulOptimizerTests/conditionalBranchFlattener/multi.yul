{
    let x := mload(0)
    let y := mload(0)			
    if slt(x, 0) {
        x := sub(0, x)
    } 
    if slt(y, 0) {
        y := add(y, 2)
    } 
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := mload(0)
//     let y := mload(0)
//     let condition := iszero(iszero(slt(x, 0)))
//     x := xor(x, and(sub(0, condition), xor(x, sub(0, x))))
//     let condition_1 := iszero(iszero(slt(y, 0)))
//     y := xor(y, and(sub(0, condition_1), xor(y, add(y, 2))))
// }
