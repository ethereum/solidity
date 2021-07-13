{
    function f(a, b) { }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:26   */
// tag_1:
//   pop
//   pop
//   jump	// out
