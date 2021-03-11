{
    let x := 0
    sstore(x,x)
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     let x := returndatasize()
//     sstore(x, x)
// }
