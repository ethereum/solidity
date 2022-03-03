{
    let y := calldataload(0)
    let t := calldataload(32)

    if sgt(sub(y, 1), y) {
        // y - 1 > y, i.e. y is the most negative value
        if eq(y, sub(0, y)) {
            sstore(0, 1)
        }
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let t := calldataload(32)
//     if sgt(sub(y, 1), y) { if 1 { sstore(0, 1) } }
// }
