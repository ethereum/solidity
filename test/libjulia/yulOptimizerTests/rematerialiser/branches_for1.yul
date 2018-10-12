{
    let a := 1
    for { pop(a) } a { pop(a) } {
        pop(a)
    }
}
// ----
// rematerialiser
// {
//     let a := 1
//     for {
//         pop(1)
//     }
//     1
//     {
//         pop(1)
//     }
//     {
//         pop(1)
//     }
// }
