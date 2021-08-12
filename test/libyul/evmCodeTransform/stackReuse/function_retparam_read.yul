{
    function f() -> x { pop(address()) sstore(0, x) pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:72   */
// tag_1:
//   0x00
//   swap1
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//   dup2
//     /* "":48:49   */
//   0x00
//     /* "":41:53   */
//   sstore
//     /* "":58:69   */
//   callvalue
//     /* "":54:70   */
//   pop
//     /* "":6:72   */
//   jump	// out
