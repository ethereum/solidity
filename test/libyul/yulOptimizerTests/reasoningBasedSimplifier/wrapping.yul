{
    let x := 7
    let y := 8
    if gt(sub(x, y), 20) { }
    if eq(sub(x, y), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff) {}
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := 7
//     let y := 8
//     if 1 { }
//     if 1 { }
// }
