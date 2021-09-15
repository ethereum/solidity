{
    function f(a, b, c) -> x { pop(address()) sstore(a, c) pop(callvalue()) x := b }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:88   */
//   stop
//     /* "":6:86   */
// tag_1:
//   swap2
//   swap1
//   swap2
//     /* "":37:46   */
//   address
//     /* "":33:47   */
//   pop
//     /* "":48:60   */
//   sstore
//     /* "":65:76   */
//   callvalue
//     /* "":61:77   */
//   pop
//     /* "":6:86   */
//   swap1
//   jump	// out
