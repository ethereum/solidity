{
    let a := caller()
    pop(a)
    for {  } a { pop(a) } {
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
//     pop(caller())
//     for { } a { pop(address()) }
//     {
//         a := address()
//         let c := address()
//     }
//     let x := a
// }
