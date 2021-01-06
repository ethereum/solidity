{
  function g() -> x { x := add(mload(x), 1) }

  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    sstore(0, a)
    let q := keccak256(g(), 32)
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let q := keccak256(g(), 32)
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     { sstore(0, a) }
//     function g() -> x
//     { x := add(mload(x), 1) }
// }
