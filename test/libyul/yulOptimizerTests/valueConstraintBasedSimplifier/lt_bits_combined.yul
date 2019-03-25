{
    let x := and(callvalue(), 0xff)
    if lt(x, 0x100) { sstore(0, 1) }
    if lt(x, 0xff) { sstore(0, 1) }
}
// ----
// valueConstraintBasedSimplifier
// x:
//     min: 0
//     max: 255
//    minB: 0
//    maxB: 255
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
