{
  let a := mload(0)
  let b := mload(1)
  a, b := f()
  sstore(a, b)
  a := mload(5)
  b := mload(a)
  function f() -> x, y {}
}
// ----
// step: ssaTransform
//
// {
//     let a_1 := mload(0)
//     let a := a_1
//     let b_1 := mload(1)
//     let b := b_1
//     let a_2, b_2 := f()
//     a := a_2
//     b := b_2
//     sstore(a_2, b_2)
//     let a_3 := mload(5)
//     a := a_3
//     let b_3 := mload(a_3)
//     b := b_3
//     function f() -> x, y
//     { }
// }
