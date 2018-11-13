{
    let b := 0
    for { let a := 1 pop(a) } lt(a, 0) { pop(a) a := add(a, 3) } {
        b := 1 pop(a)
    }
}
// ----
// rematerialiser
// {
//     let b := 0
//     for {
//         let a := 1
//         pop(1)
//     }
//     lt(a, 0)
//     {
//         pop(a)
//         a := add(a, 3)
//     }
//     {
//         b := 1
//         pop(a)
//     }
// }
