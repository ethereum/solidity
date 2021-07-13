{
    function f(a, b, c, d) -> x, y { b := 3 let s := 9 y := 2 mstore(s, y) }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":6:78   */
// tag_1:
//   pop
//   pop
//   pop
//   pop
//   0x00
//   swap1
//     /* "":44:45   */
//   pop(0x03)
//     /* "":55:56   */
//   0x09
//   swap1
//     /* "":62:63   */
//   0x02
//   dup1
//   swap3
//     /* "":64:76   */
//   mstore
//     /* "":6:78   */
//   jump	// out
