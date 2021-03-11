{
    let x := 0
    let y := call(0, 0, 0, 0, 0, 0, 0)
    sstore(y, 0)
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     let x := returndatasize()
//     let y := call(returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize(), returndatasize())
//     sstore(y, 0)
// }
