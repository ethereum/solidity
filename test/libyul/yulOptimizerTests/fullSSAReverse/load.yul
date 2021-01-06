{
    phi_store("a", 23)
    let a := 42
    sstore(0, phi_load("a"))
}
// ----
// step: fullSSAReverse
//
// {
//     let a_1 := 23
//     let a := 42
//     sstore(0, a_1)
// }
