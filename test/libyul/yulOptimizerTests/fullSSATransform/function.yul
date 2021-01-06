{
  function f(a, b) -> c {
    c := calldataload(0)
    sstore(0, a)
    a := b
    sstore(1, b)
    c := calldataload(1)
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
//         sstore(0, a)
//         let a_2 := b
//         sstore(1, b)
//         let c_3 := calldataload(1)
//         phi_store("c", c_3)
//     }
//     sstore(2, f(calldataload(2), calldataload(3)))
// }
