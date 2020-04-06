{
    let y:i32 := 0:i32
    for {} true { } {
        if y { break }
    }
}
// ====
// dialect: ewasm
// ----
// step: conditionalSimplifier
//
// {
//     let y:i32 := 0:i32
//     for { } true { }
//     {
//         if y { break }
//         y := false
//     }
// }
