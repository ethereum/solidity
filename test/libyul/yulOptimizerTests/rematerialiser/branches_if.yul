{
    let a := 1
    let b := 2
    if b { pop(b) b := a }
    let c := b
}
// ----
// rematerialiser
// {
//     let a := 1
//     let b := 2
//     if 2
//     {
//         pop(2)
//         b := 1
//     }
//     let c := b
// }
