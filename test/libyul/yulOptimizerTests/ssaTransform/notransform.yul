{
  let a := 1
  // this should not be transformed
  let b := add(a, 2)
  let c
  mstore(c, 0)
  c := add(a, b)
}
// ====
// step: ssaTransform
// ----
// {
//     let a := 1
//     let b := add(a, 2)
//     let c_1
//     let c := c_1
//     mstore(c_1, 0)
//     let c_2 := add(a, b)
//     c := c_2
// }
