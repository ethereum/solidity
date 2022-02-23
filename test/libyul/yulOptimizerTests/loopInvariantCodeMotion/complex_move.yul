{
  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let inv := add(b, 42)
    let x := extcodesize(keccak256(mul(mload(inv), 3), 32))
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
//     let x := extcodesize(keccak256(mul(mload(inv), 3), 32))
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         a := add(x, 1)
//         sstore(a, inv)
//     }
// }
