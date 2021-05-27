{
    sstore(4, 5)
    sstore(4, 3)
    sstore(8, sload(4))
}
// ----
// step: fullSuite
//
// {
//     {
//         sstore(4, 3)
//         sstore(8, 3)
//     }
// }
