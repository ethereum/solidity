{
    function f() -> x { pop(address()) for { pop(callvalue()) } 0 {} { } }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:76   */
// tag_1:
//   0x00
//   swap1
//     /* "":30:39   */
//   address
//     /* "":26:40   */
//   pop
//     /* "":51:62   */
//   callvalue
//     /* "":47:63   */
//   pop
//     /* "":6:76   */
//   jump	// out
