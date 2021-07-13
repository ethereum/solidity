{
    function f() -> x, y, z { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:50   */
// tag_1:
//   0x00
//   0x00
//   0x00
//   swap3
//     /* "":36:47   */
//   callvalue
//     /* "":32:48   */
//   pop
//     /* "":6:50   */
//   jump	// out
