{
    let x := mload(0)
    if x { sstore(0, x) revert(0, 0) }
    x := 0
    sstore(1, x)
}
// ====
// step: conditionalUnsimplifier
// ----
// {
//     let x := mload(0)
//     if x
//     {
//         sstore(0, x)
//         revert(0, 0)
//     }
//     sstore(1, x)
// }
