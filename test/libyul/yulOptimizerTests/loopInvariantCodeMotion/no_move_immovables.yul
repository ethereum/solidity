{
  let a := 1
  function f() -> x {invalid()}
  function g() -> y {return(0, 0)}
  for { let i := 1 } iszero(eq(i, 10)) { a := add(i, 1) } {
    let b := f()
    let c := gas()
    let d := g()
    let e := sload(g())
  }
}
// ----
// step: loopInvariantCodeMotion
//
// {
//     let a := 1
//     let i := 1
//     for { } iszero(eq(i, 10)) { a := add(i, 1) }
//     {
//         let b := f()
//         let c := gas()
//         let d := g()
//         let e := sload(g())
//     }
//     function f() -> x
//     { invalid() }
//     function g() -> y
//     { return(0, 0) }
// }
