{
    let y:bool := false
    for {} true { } {
        if y { break }
    }
}
// ====
// dialect: evmTyped
// ----
// step: conditionalSimplifier
//
// {
//     let y:bool := false
//     for { } true { }
//     {
//         if y { break }
//         y := false
//     }
// }
