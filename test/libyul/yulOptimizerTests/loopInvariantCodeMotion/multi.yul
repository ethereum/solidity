{
  let b := 1
  // tests if c, d, and inv can be moved outside in single pass
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let c := b
    let d := mul(c, 2)
    let inv := add(c, d)
    a := add(a, 1)
    mstore(a, inv)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let c := b
//     let d := mul(c, 2)
//     let inv := add(c, d)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         a := add(a, 1)
//         mstore(a, inv)
//     }
// }
