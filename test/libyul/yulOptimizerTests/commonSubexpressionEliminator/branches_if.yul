{
    let b := 1
    if b { b := 1 }
    let c := 1
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let b := 1
//     if b
//     {
//         b := b
//     }
//     let c := 1
// }
