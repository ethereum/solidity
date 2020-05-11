{
    let x := calldataload(2)
    let t := lt(x, 20)
    if t {
        if lt(x, 21) { }
        if lt(x, 20) { }
        if lt(x, 19) { }
        if gt(x, 20) { }
        if iszero(gt(x, 20)) { }
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := calldataload(2)
//     let t := lt(x, 20)
//     if t
//     {
//         if 1 { }
//         if 1 { }
//         if lt(x, 19) { }
//         if 0 { }
//         if 1 { }
//     }
// }
