{
    function f() -> x { pop(address()) if 1 { pop(callvalue()) } }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:68   */
// tag_1:
//   0x00
//   swap1
//     /* "":44:45   */
//   0x01
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//   tag_2
//   jumpi
// tag_3:
//     /* "":6:68   */
//   jump	// out
// tag_2:
//     /* "":52:63   */
//   callvalue
//     /* "":48:64   */
//   pop
//   jump(tag_3)
