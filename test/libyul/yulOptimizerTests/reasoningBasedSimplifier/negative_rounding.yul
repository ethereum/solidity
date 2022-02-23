{
    let x := sub(0, 7)
    let y := 2
    // (-7)/2 == -3 on the evm
    if iszero(add(sdiv(x, y), 3)) { }
    if iszero(add(sdiv(x, y), 4)) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := sub(0, 7)
//     let y := 2
//     if 1 { }
//     if 0 { }
// }
