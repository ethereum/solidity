{
  function g() -> x { x := add(sload(mload(x)), 1) }

  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let t := mload(g())
    let s := keccak256(g(), 32)
    let q := g()
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let b := 1
//     let a := 1
//     let t := mload(g())
//     let s := keccak256(g(), 32)
//     let q := g()
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     { }
//     function g() -> x
//     { x := add(sload(mload(x)), 1) }
// }
