{
    let b := 0
    for { let a := caller() pop(a) } a { pop(a) } {
        b := 1 pop(a)
    }
}
// ----
// rematerialiser
// {
//     let b := 0
//     for {
//         let a := caller()
//         pop(caller())
//     }
//     caller()
//     {
//         pop(caller())
//     }
//     {
//         b := 1
//         pop(caller())
//     }
// }
