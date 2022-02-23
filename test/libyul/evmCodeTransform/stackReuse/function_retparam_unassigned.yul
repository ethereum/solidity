{
    function f() -> x { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:46   */
//   stop
//     /* "":6:44   */
// tag_1:
//     /* "":22:23   */
//   0x00
//     /* "":6:44   */
//   swap1
//     /* "":30:41   */
//   callvalue
//     /* "":26:42   */
//   pop
//     /* "":6:44   */
//   jump	// out
