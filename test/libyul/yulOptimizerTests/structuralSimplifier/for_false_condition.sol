{
    for { let a := 42 } 0 { a := a } {
        let b := a
    }
}
// ----
// structuralSimplifier
// {
//     let a := 42
// }
