{
  function f() -> x { x := g() }
  function g() -> x {
    x := add(x, 1)
    sstore(0x00, 0x00)
  }

  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let t := extcodesize(f())
    let q := sload(g())
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
//         let t := extcodesize(f())
//         let q := sload(g())
//     }
//     function f() -> x
//     { x := g() }
//     function g() -> x_1
//     {
//         x_1 := add(x_1, 1)
//         sstore(0x00, 0x00)
//     }
// }
