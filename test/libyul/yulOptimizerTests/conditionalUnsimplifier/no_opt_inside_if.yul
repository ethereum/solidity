{
    let x := mload(0)
    if x { sstore(0, x) }
    x := 0
    sstore(1, x)
}
// ====
// step: conditionalUnsimplifier
// ----
// {
//     let x := mload(0)
//     if x { sstore(0, x) }
//     x := 0
//     sstore(1, x)
// }
