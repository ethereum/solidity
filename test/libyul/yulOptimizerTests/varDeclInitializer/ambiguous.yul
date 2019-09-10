{
  // This component does not need the disambiguator
  function f() -> x, y {
    let a, b
    mstore(a, b)
    let d
    d := 2
  }
  let a
  a := 4
  let b := 2
  let x, y := f()
}
// ====
// step: varDeclInitializer
// ----
// {
//     function f() -> x, y
//     {
//         let a := 0
//         let b := 0
//         mstore(a, b)
//         let d := 0
//         d := 2
//     }
//     let a := 0
//     a := 4
//     let b := 2
//     let x, y := f()
// }
