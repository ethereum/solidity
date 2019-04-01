{
    for { let a := 42 } 0 { a := a } {
        let b := a
    }
}
// ====
// step: structuralSimplifier
// ----
// {
//     let a := 42
// }
