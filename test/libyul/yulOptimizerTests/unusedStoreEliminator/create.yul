{
    sstore(5, 10)
    pop(create(0, 0, 0))
    sstore(5, 20)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         sstore(5, 10)
//         pop(create(0, 0, 0))
//         sstore(5, 20)
//     }
// }
