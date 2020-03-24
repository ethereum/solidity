{
    let y:bool := false
    for {} true { } {
        if y { break }
    }
}
// ====
// dialect: yul
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
