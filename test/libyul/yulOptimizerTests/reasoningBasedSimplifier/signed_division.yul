{
    let y := calldataload(0)
    let t := calldataload(32)

    if sgt(sub(y, 1), y) {
        // y - 1 > y, i.e. y is the most negative value
        if eq(sdiv(y, sub(0, 1)), y) {
            // should be true: y / -1 == y
            sstore(0, 7)
        }
        if iszero(eq(y, t)) {
            // t is not the most negative value
            if eq(sdiv(t, sub(0, 1)), sub(0, t)) {
                // should be true: t / -1 = 0 - t
                sstore(1, 7)
            }
        }
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let t := calldataload(32)
//     if sgt(sub(y, 1), y)
//     {
//         if 1 { sstore(0, 7) }
//         if iszero(eq(y, t)) { if 1 { sstore(1, 7) } }
//     }
// }
