{
    let a := 1
    let b := 2
    switch number()
    case 1 { b := a }
    default { let x := a let y := b b := a }
    pop(add(a, b))
}
// ====
// step: rematerialiser
// ----
// {
//     let a := 1
//     let b := 2
//     switch number()
//     case 1 { b := 1 }
//     default {
//         let x := 1
//         let y := b
//         b := 1
//     }
//     pop(add(1, b))
// }
