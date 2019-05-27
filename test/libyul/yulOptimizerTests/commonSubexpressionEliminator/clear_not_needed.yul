{
    let a := calldataload(0)
    let x := calldataload(0x20)
    x := a
    let z := 0
    x := z
    a := 9
    sstore(x, 3)
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let a := calldataload(0)
//     let x := calldataload(0x20)
//     x := a
//     let z := 0
//     x := z
//     a := 9
//     sstore(z, 3)
// }
