{
    let a := 1
    for { pop(a) } a { pop(a) } {
        a := 7
        let c := a
    }
    let x := a
}
// ----
// rematerialiser
// {
//     let a := 1
//     for {
//         pop(1)
//     }
//     a
//     {
//         pop(7)
//     }
//     {
//         a := 7
//         let c := 7
//     }
//     let x := a
// }
