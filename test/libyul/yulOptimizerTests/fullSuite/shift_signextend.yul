{
    let x := calldataload(0)
    let a := shl(sub(256, 8), signextend(0, x))
    sstore(0, a)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: fullSuite
//
// {
//     {
//         sstore(0, shl(248, calldataload(0)))
//     }
// }
