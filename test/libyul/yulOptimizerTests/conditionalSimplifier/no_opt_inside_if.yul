{
    let x := mload(0)
    if x { sstore(0, x) }
    sstore(1, x)
}
// ====
// step: conditionalSimplifier
// ----
// {
//     let x := mload(0)
//     if x { sstore(0, x) }
//     sstore(1, x)
// }
