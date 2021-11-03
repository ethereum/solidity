{
    function f() -> x { pop(address()) for { pop(callvalue()) } 0 {} { } }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:78   */
//   stop
//     /* "":6:76   */
// tag_1:
//     /* "":22:23   */
//   0x00
//     /* "":6:76   */
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
