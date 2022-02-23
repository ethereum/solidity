{
    let x := 7
    let y := 8
    if eq(add(x, y), 15) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     let x := 7
//     let y := 8
//     if 1 { }
// }
