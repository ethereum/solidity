{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    for { let a2 := 1 } iszero(eq(a2, 10)) { a2 := add(a2, 1) } {
      let inv := add(b, 42)
      mstore(a, inv)
    }
    a := add(a, 1)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let inv := add(b, 42)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let a2 := 1
//         for { } iszero(eq(a2, 10)) { a2 := add(a2, 1) }
//         { mstore(a, inv) }
//         a := add(a, 1)
//     }
// }
