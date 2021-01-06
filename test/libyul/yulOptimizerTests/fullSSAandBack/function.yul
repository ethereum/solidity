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
// step: fullSSAandBack
//
// {
//     function f(a, b) -> c_4
//     {
//         sstore(0, a)
//         sstore(1, b)
//         c_4 := calldataload(1)
//     }
//     sstore(2, f(calldataload(2), calldataload(3)))
// }
