{
    // 7 % 5 == 2
    // 7 % -5 == 2
    // -7 % 5 == -2
    // -7 % -5 == -2
    // -5 % -5 == 0
    let x := calldataload(0)
    let y := calldataload(32)
    let result := smod(x, y)
    if eq(x, 7) {
        if eq(y, 5) {
            if eq(result, 2) { sstore(0, 7)}
        }
        if eq(y, sub(0, 5)) {
            if eq(result, 2) { sstore(0, 7)}
        }
    }
    if eq(x, sub(0, 7)) {
        if eq(y, 5) {
            if eq(result, sub(0, 2)) { sstore(0, 7)}
        }
        if eq(y, sub(0, 5)) {
            if eq(result, sub(0, 2)) { sstore(0, 7)}
        }
    }
    if eq(x, sub(0, 5)) {
        if eq(y, sub(0, 5)) {
            if eq(result, 0) { sstore(0, 7)}
        }
    }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := calldataload(0)
//     let y := calldataload(32)
//     let result := smod(x, y)
//     if eq(x, 7)
//     {
//         if eq(y, 5) { if 1 { sstore(0, 7) } }
//         if eq(y, sub(0, 5)) { if 1 { sstore(0, 7) } }
//     }
//     if eq(x, sub(0, 7))
//     {
//         if eq(y, 5) { if 1 { sstore(0, 7) } }
//         if eq(y, sub(0, 5)) { if 1 { sstore(0, 7) } }
//     }
//     if eq(x, sub(0, 5))
//     {
//         if eq(y, sub(0, 5)) { if 1 { sstore(0, 7) } }
//     }
// }
