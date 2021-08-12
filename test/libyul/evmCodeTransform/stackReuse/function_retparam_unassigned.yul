{
    function f() -> x { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:44   */
// tag_1:
//   0x00
//   swap1
//     /* "":30:41   */
//   callvalue
//     /* "":26:42   */
//   pop
//     /* "":6:44   */
//   jump	// out
