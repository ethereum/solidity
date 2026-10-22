{
    let x := 10
    let c := 1
    if c { x := add(x, 1) }
}
// ----
// step: conditionalBranchFlattener
//
// {
//     let x := 10
//     let c := 1
//     let condition := iszero(iszero(c))
//     x := xor(x, and(sub(0, condition), xor(x, add(x, 1))))
// }
