{
    function f() -> x { pop(address()) leave pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:67   */
//   stop
//     /* "":6:65   */
// tag_1:
//     /* "":22:23   */
//   0x00
//     /* "":6:65   */
//   swap1
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//     /* "":41:46   */
//   jump	// out
