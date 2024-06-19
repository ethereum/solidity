{
  let x, y := f(1, 2)
  x := mload(y)
  y := mload(x)
  let a, b := f(x, y)
  sstore(a, b)
  function f(t, v) -> w, z {}
}
// ----
// step: ssaTransform
//
// {
//     let x_1, y_1 := f(1, 2)
//     let x := x_1
//     let y := y_1
//     let x_2 := mload(y_1)
//     x := x_2
//     let y_2 := mload(x_2)
//     y := y_2
//     let a, b := f(x_2, y_2)
//     sstore(a, b)
//     function f(t, v) -> w, z
//     { }
// }
