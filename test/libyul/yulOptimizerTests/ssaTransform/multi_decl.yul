{
  let x, y := f(1, 2)
  x := mload(y)
  y := mload(x)
  let a, b := f(x, y)
  sstore(a, b)
  function f(t, v) -> x, y {}
}
// ====
// step: ssaTransform
// ----
// {
//     let x_2, y_3 := f(1, 2)
//     let x := x_2
//     let y := y_3
//     let x_4 := mload(y_3)
//     x := x_4
//     let y_5 := mload(x_4)
//     y := y_5
//     let a, b := f(x_4, y_5)
//     sstore(a, b)
//     function f(t, v) -> x_1, y_2
//     {
//     }
// }
