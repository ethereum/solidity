{
    function f() -> x { pop(address()) leave pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:65   */
// tag_1:
//   0x00
//   swap1
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//     /* "":6:65   */
//   jump	// out
