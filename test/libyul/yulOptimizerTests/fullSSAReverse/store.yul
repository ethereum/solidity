{
    phi_store("a", 42)
    let b := 0
    phi_store("b", 23)
}
// ----
// step: fullSSAReverse
//
// {
//     let a_1 := 42
//     let b := 0
//     let b_2 := 23
// }
