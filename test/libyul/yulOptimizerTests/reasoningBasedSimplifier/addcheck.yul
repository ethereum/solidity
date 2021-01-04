{
    let x := calldataload(0)
    let y := calldataload(1)
    if gt(x, 200) { revert(0, 0) }
    if gt(y, 600) { revert(0, 0) }
    if gt(x, not(y)) { revert(0, 0) }
    sstore(0, add(x, y))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := calldataload(0)
//     let y := calldataload(1)
//     if gt(x, 200) { revert(0, 0) }
//     if gt(y, 600) { revert(0, 0) }
//     if 0 { }
//     sstore(0, add(x, y))
// }
