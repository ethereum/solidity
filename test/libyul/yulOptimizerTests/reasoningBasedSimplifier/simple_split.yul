{
    let y := calldataload(0)
    let x := calldataload(1)
    let _1 := lt(x, y)
    let _2 := not(x)
    if and(_1, _2) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let x := calldataload(1)
//     let _1 := lt(x, y)
//     let _2 := not(x)
//     if and(_1, _2) { }
// }
