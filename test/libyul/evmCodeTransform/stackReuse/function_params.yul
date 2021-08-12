{
    function f(a, b) { }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:28   */
//   stop
//     /* "":6:26   */
// tag_1:
//   pop
//   pop
//   jump	// out
