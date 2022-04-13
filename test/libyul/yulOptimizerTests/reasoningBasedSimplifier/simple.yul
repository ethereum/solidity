{
    let y := calldataload(0)
    let x := calldataload(1)
    if and(lt(x, y), iszero(not(x))) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let x := calldataload(1)
//     if 0 { }
// }
