{
  let a := 42
  if calldataload(42) { a := 23 }
  sstore(0, 42)
}
// ----
// step: fullSSATransform
//
// {
//     let a := 42
//     phi_store("a", a)
//     if calldataload(42)
//     {
//         let a_1 := 23
//         phi_store("a", a_1)
//     }
//     let a_2 := phi_load("a")
//     sstore(0, 42)
// }
