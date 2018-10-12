{
    let a := add(mul(calldatasize(), 2), number())
    let b := add(a, a)
}
// ----
// rematerialiser
// {
//     let a := add(mul(calldatasize(), 2), number())
//     let b := add(add(mul(calldatasize(), 2), number()), add(mul(calldatasize(), 2), number()))
// }
