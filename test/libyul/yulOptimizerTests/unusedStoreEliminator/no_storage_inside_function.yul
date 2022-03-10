{
  function f() -> r {
     r := mload(0x20)
  }
  let x := 5
  sstore(x, 10) // should be removed
  mstore(0, 42) // could be removed, but will probably stay?
  pop(f())
  sstore(x, 10)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let x := 5
//         let _2 := 10
//         mstore(0, 42)
//         pop(f())
//         sstore(x, 10)
//     }
//     function f() -> r
//     {
//         r := mload(0x20)
//         let r_7 := r
//     }
// }
