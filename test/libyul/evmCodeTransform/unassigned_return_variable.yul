{
  // This used to throw during stack layout generation.
  function g(b,s) -> y {
    y := g(b, g(y, s))
  }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:111   */
//   stop
//     /* "":60:109   */
// tag_1:
//   pop
//     /* "":99:100   */
//   0x00
//     /* "":97:104   */
//   tag_1
//   jump	// in
