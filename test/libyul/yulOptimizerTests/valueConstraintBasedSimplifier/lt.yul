{
    let x := 2
    if lt(x, 9) { sstore(0, 1) }
    if lt(x, 2) { sstore(0, 1) }
    if gt(x, 0) { sstore(0, 1) }
    if lt(9, x) { sstore(0, 1) }
    if lt(2, x) { sstore(0, 1) }
    if gt(0, x) { sstore(0, 1) }
}
// ----
// valueConstraintBasedSimplifier
// x:
//        = 2
// {
//     let x := 2
//     if 1
//     {
//         sstore(0, 1)
//     }
//     if 0
//     {
//         sstore(0, 1)
//     }
//     if 1
//     {
//         sstore(0, 1)
//     }
//     if 0
//     {
//         sstore(0, 1)
//     }
//     if 0
//     {
//         sstore(0, 1)
//     }
//     if 0
//     {
//         sstore(0, 1)
//     }
// }
