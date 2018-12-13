{
  function f() -> x, y {
    let a, b
    mstore(a, b)
    let d
    d := 2
  }
  let r
  r := 4
}
// ----
// varDeclInitializer
// {
//     function f() -> x, y
//     {
//         let a := 0
//         let b := 0
//         mstore(a, b)
//         let d := 0
//         d := 2
//     }
//     let r := 0
//     r := 4
// }
