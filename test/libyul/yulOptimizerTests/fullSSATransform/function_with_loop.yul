{
  function f(a, b) -> c {
    c := calldataload(0)
    for {} gt(a,0) { a := sub(a,b) } {
      c := calldataload(a)
    }
    c := calldataload(c)
  }
  sstore(2, f(calldataload(2), calldataload(3)))
}
// ----
// step: fullSSATransform
//
// {
//     function f(a, b) -> c
//     {
//         let c_1 := calldataload(0)
//         phi_store("c_1", c_1)
//         phi_store("a", a)
//         for { }
//         true
//         {
//             let c_5 := phi_load("c_1")
//             let a_6 := phi_load("a")
//             let a_7 := sub(a_6, b)
//             phi_store("c_1", c_5)
//             phi_store("a", a_7)
//         }
//         {
//             let c_2 := phi_load("c_1")
//             let a_3 := phi_load("a")
//             if iszero(gt(a_3, 0))
//             {
//                 phi_store("c_1", c_2)
//                 phi_store("a", a_3)
//                 break
//             }
//             let c_4 := calldataload(a_3)
//             phi_store("c_1", c_4)
//             phi_store("a", a_3)
//         }
//         let c_8 := phi_load("c_1")
//         let a_9 := phi_load("a")
//         let c_10 := calldataload(c_8)
//         phi_store("c", c_10)
//     }
//     sstore(2, f(calldataload(2), calldataload(3)))
// }
