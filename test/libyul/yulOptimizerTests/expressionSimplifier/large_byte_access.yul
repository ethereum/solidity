{
    let a := calldataload(0)
    let b := byte(33, a)
    let c := byte(20, a)
    // create cannot be removed.
    let d := byte(33, create(0, 0, 0x20))
}
// ====
// step: expressionSimplifier
// ----
// {
//     let a := calldataload(0)
//     let b := 0
//     let c := byte(20, a)
//     let d := byte(33, create(0, 0, 0x20))
// }
