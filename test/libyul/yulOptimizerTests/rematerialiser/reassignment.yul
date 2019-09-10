{
    let a := 1
    pop(a)
    if a { a := 2 }
    let b := mload(a)
    pop(b)
}
// ====
// step: rematerialiser
// ----
// {
//     let a := 1
//     pop(1)
//     if 1 { a := 2 }
//     let b := mload(a)
//     pop(b)
// }
