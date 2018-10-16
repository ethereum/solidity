{
  let a := 1
  mstore(a, 0)
  a := 2
  mstore(a, 0)
  {
    mstore(a, 0)
    a := 3
    mstore(a, 0)
    a := 4
    mstore(a, 0)
  }
  mstore(a, 0)
  a := 4
  mstore(a, 0)
}
// ----
// ssaTransform
// {
//     let a_1 := 1
//     let a := a_1
//     mstore(a_1, 0)
//     let a_2 := 2
//     a := a_2
//     mstore(a_2, 0)
//     {
//         mstore(a_2, 0)
//         let a_3 := 3
//         a := a_3
//         mstore(a_3, 0)
//         let a_4 := 4
//         a := a_4
//         mstore(a_4, 0)
//     }
//     mstore(a, 0)
//     let a_5 := 4
//     a := a_5
//     mstore(a_5, 0)
// }
