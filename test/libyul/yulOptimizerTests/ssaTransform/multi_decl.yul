{
  let x, y := f(1, 2)
  x := mload(y)
  y := mload(x)
  let a, b := f(x, y)
  sstore(a, b)
  function f(t, v) -> w, z {}
}
// ====
// step: ssaTransform
// ----
// {
//     let x_1, y_2 := f(1, 2)
//     let x := x_1
//     let y := y_2
//     let x_3 := mload(y_2)
//     x := x_3
//     let y_4 := mload(x_3)
//     y := y_4
//     let a, b := f(x_3, y_4)
//     sstore(a, b)
//     function f(t, v) -> w, z
//     {
//     }
// }
