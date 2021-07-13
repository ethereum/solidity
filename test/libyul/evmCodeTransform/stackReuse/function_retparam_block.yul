{
    function f() -> x { pop(address()) { pop(callvalue()) } }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:63   */
// tag_1:
//   0x00
//   swap1
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//     /* "":47:58   */
//   callvalue
//     /* "":43:59   */
//   pop
//     /* "":6:63   */
//   jump	// out
