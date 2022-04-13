{
    let y := calldataload(0)
    let x := calldataload(1)
    if and(lt(x, y), eq(x, sub(0, 1))) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let x := calldataload(1)
//     if 0 { }
// }
