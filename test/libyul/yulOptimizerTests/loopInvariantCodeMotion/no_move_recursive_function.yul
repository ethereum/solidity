{
  function f() -> x { x := g() }
  function g() -> x { x := g() }

  let b := 1
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    let t := f()
    let q := g()
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
//         let t := f()
//         let q := g()
//     }
//     function f() -> x
//     { x := g() }
//     function g() -> x_1
//     { x_1 := g() }
// }
