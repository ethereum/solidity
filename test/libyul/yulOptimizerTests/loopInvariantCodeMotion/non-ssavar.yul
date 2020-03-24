{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let not_inv := add(b, 42)
    not_inv := add(not_inv, 1)
    a := add(a, 1)
    mstore(a, not_inv)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let not_inv := add(b, 42)
//         not_inv := add(not_inv, 1)
//         a := add(a, 1)
//         mstore(a, not_inv)
//     }
// }
