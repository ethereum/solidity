{
  function f(a, b) -> c {
    c := calldataload(0)
    for {} gt(a,0) { a := sub(a,b) } {
      c := calldataload(a)
      if lt(c, add(b, a)) {
        leave
      }
    }
    c := calldataload(c)
  }
  sstore(2, f(calldataload(2), calldataload(3)))
}
// ----
// step: fullSSAandBack
//
// {
//     function f(a, b) -> c_11
//     {
//         let c_1_12 := calldataload(0)
//         let a_13 := a
//         for { } gt(a_13, 0) { a_13 := sub(a_13, b) }
//         {
//             let c_4 := calldataload(a_13)
//             if lt(c_4, add(b, a_13))
//             {
//                 c_11 := c_4
//                 leave
//             }
//             c_1_12 := c_4
//         }
//         c_11 := calldataload(c_1_12)
//     }
//     sstore(2, f(calldataload(2), calldataload(3)))
// }
