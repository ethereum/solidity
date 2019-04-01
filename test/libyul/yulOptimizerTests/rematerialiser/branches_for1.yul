{
    let a := caller()
    for { pop(a) } a { pop(a) } {
        pop(a)
    }
}
// ====
// step: rematerialiser
// ----
// {
//     let a := caller()
//     for {
//         pop(caller())
//     }
//     caller()
//     {
//         pop(caller())
//     }
//     {
//         pop(caller())
//     }
// }
