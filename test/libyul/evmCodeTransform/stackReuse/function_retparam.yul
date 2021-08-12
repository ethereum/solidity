{
    function f() -> x, y { }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:30   */
// tag_1:
//     /* "":25:26   */
//   0x00
//     /* "":22:23   */
//   0x00
//   swap2
//     /* "":6:30   */
//   jump	// out
