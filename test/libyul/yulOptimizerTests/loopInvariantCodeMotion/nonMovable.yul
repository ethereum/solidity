{
  let b := 0
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := mload(b)
    a := add(a, 1)
    mstore(a, inv)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 0
//     let a := 1
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let inv := mload(b)
//         a := add(a, 1)
//         mstore(a, inv)
//     }
// }
