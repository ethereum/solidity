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
//   swap1
//   0x00
//   swap1
//   0x00
//   swap1
//     /* "":36:47   */
//   callvalue
//     /* "":32:48   */
//   pop
//     /* "":6:50   */
//   jump	// out
