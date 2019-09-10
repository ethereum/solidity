{
  let a := mload(0)
  let b := mload(1)
  a, b := f()
  sstore(a, b)
  a := mload(5)
  b := mload(a)
  function f() -> x, y {}
}
// ====
// step: ssaTransform
// ----
// {
//     let a_1 := mload(0)
//     let a := a_1
//     let b_2 := mload(1)
//     let b := b_2
//     let a_3, b_4 := f()
//     a := a_3
//     b := b_4
//     sstore(a_3, b_4)
//     let a_5 := mload(5)
//     a := a_5
//     let b_6 := mload(a_5)
//     b := b_6
//     function f() -> x, y
//     { }
// }
