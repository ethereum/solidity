{
    let b := 0
    for { let a := 1 pop(a) } a { pop(a) } {
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
//     1
//     {
//         pop(1)
//     }
//     {
//         b := 1
//         pop(1)
//     }
// }
