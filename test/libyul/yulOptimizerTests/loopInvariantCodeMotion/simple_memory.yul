{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := mload(mul(inv, 3))
    let y := keccak256(mul(b, 13), 32)
    a := add(x, 1)
    sstore(a, inv)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let inv := add(b, 42)
//     let x := mload(mul(inv, 3))
//     let y := keccak256(mul(b, 13), 32)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         a := add(x, 1)
//         sstore(a, inv)
//     }
// }
