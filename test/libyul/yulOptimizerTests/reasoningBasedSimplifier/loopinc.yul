{
    let y := calldataload(0)
    let sum := 0
    let x := 0
    for {} lt(x, y) { } {
        // overflow check
        if not(x) { revert(0, 0) }
        // different way to do overflow check
        if lt(add(x, 1), x) { revert(0, 0) }
        sum := calldataload(add(0x20, mul(x, 0x20)))
        x := add(x, 1)
    }
    sstore(0, sum)
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let y := calldataload(0)
//     let t := calldataload(32)
//     if sgt(sub(y, 1), y) { if 1 { sstore(0, 1) } }
// }
