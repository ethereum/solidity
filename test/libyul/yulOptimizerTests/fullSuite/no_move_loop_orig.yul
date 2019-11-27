{
  for {} msize() {
    function foo_s_0() -> x_1 { for {} caller() {} {} }
    // x_3 used to be a movable loop invariant because `foo_s_0()` used to be movable
    let x_3 := foo_s_0()
    mstore(192, x_3)
  }
  {}
}
// ====
// step: fullSuite
// ----
// {
//     {
//         let _1 := iszero(caller())
//         for { }
//         1
//         {
//             for { } iszero(_1) { }
//             { }
//             mstore(192, 0)
//         }
//         { if iszero(msize()) { break } }
//     }
// }
