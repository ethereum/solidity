{
    let a := extcodesize(0)
    let b := a
    let c := b
    a := 2
    let d := add(b, c)
    pop(a) pop(b) pop(c) pop(d)
}
// ----
// rematerialiser
// {
//     let a := extcodesize(0)
//     let b := a
//     let c := a
//     a := 2
//     let d := add(b, c)
//     pop(2)
//     pop(b)
//     pop(c)
//     pop(add(b, c))
// }
