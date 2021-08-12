{
    function f() -> x, y { }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:32   */
//   stop
//     /* "":6:30   */
// tag_1:
//     /* "":25:26   */
//   0x00
//     /* "":22:23   */
//   0x00
//     /* "":6:30   */
//   swap2
//   jump	// out
