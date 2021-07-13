{
    function f() -> x, y { }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:30   */
// tag_1:
//   0x00
//   0x00
//   swap2
//   jump	// out
