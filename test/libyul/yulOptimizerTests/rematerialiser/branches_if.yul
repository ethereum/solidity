{
    let a := caller()
    let b := address()
    if b { pop(b) b := a }
    let c := b
}
// ====
// step: rematerialiser
// ----
// {
//     let a := caller()
//     let b := address()
//     if address()
//     {
//         pop(address())
//         b := caller()
//     }
//     let c := b
// }
