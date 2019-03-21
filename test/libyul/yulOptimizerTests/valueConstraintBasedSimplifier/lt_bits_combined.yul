{
    let x := and(callvalue(), 0xff)
    if lt(x, 0x100) { sstore(0, 1) }
    if lt(x, 0xff) { sstore(0, 1) }
}
// ----
// valueConstraintBasedSimplifier
// {
//     let x := and(callvalue(), 0xff)
//     if 1
//     {
//         sstore(0, 1)
//     }
//     if lt(x, 0xff)
//     {
//         sstore(0, 1)
//     }
// }
