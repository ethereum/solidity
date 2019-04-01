{
    let a := caller()
    for { pop(a) } a { pop(a) } {
        a := address()
        let c := a
    }
    let x := a
}
// ====
// step: rematerialiser
// ----
// {
//     let a := caller()
//     for {
//         pop(caller())
//     }
//     a
//     {
//         pop(address())
//     }
//     {
//         a := address()
//         let c := address()
//     }
//     let x := a
// }
