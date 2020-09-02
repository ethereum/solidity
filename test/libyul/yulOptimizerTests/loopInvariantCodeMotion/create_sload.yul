{
  function g() -> x { x := create(100, 0, 32) }
  function f() -> x { x := mload(0) }

  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    // cannot be moved because of the create call in g()
    let q := sload(5)
    let r := g()
    // This can be moved
    let z := f()
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     function g() -> x
//     { x := create(100, 0, 32) }
//     function f() -> x_1
//     { x_1 := mload(0) }
//     let b := 1
//     let a := 1
//     let z := f()
//     for { } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         let q := sload(5)
//         let r := g()
//     }
// }
