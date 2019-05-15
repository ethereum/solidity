{
  let x := 8
  let y := 0xffff
  for { let i := 0xff00 } lt(i, 2) { i := add(i, 3) } {
  }
}
// ====
// step: constantOptimiser
// ----
// {
//     let x := 8
//     let y := 0xffff
//     for { let i := 0xff00 } lt(i, 2) { i := add(i, 3) }
//     { }
// }
