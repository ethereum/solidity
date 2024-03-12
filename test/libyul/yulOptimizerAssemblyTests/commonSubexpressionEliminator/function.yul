{
    function f(a, b)
    {
    }
    let x := /** @debug.set {"id": 0} */ 42 /** @debug.set {} */
    let y := /** @debug.set {"id":" 1"} */ add(sload(42), calldataload(0)) /** @debug.set {} */
    let z := /** @debug.set {"id":"2"} */ 42 /** @debug.set {} */
    f(/** @debug.set {"f": 0} */ 42, /** @debug.set {"f": 1} */add(sload(42), /** @debug.set {"f": 2} */calldataload(0))) /** @debug.set {} */
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let x := /** @debug.set {"id":0} */ 42
//     let y := /** @debug.set {"id":" 1"} */ add(sload(x), /** @debug.set {"id":" 1"} */ calldataload(0))
//     let z := x
//     f(x, /** @debug.set {"f":1} */ add(sload(x), /** @debug.set {"f":2} */ calldataload(0)))
//     function f(a, b)
//     { }
// }
//
// Assembly:
//     /* "":76:78   */
//   0x2a   // @debug.set {"id":0}
//     /* "":171:172   */
//   0x00   // @debug.set {"id":" 1"}
//     /* "":158:173   */
//   calldataload   // @debug.set {"id":" 1"}
//     /* "":153:155   */
//   dup2   // @debug.set [{"id":0},{"id":" 1"}]
//     /* "":147:156   */
//   sload   // @debug.set {"id":" 1"}
//     /* "":143:174   */
//   add   // @debug.set {"id":" 1"}
//     /* "":238:240   */
//   dup2   // @debug.set [{"id":0},{"id":"2"}]
//     /* "":266:383   */
//   tag_2
//     /* "":379:380   */
//   0x00   // @debug.set {"f":2}
//     /* "":366:381   */
//   calldataload   // @debug.set {"f":2}
//     /* "":335:337   */
//   dup5   // @debug.set [{"id":0},{"f":1}]
//     /* "":329:338   */
//   sload   // @debug.set {"f":1}
//     /* "":325:382   */
//   add   // @debug.set {"f":1}
//     /* "":295:297   */
//   dup5   // @debug.set [{"id":0},{"f":0}]
//     /* "":266:383   */
//   tag_1
//   jump	// in
// tag_2:
//     /* "":6:34   */
//   jump(tag_3)
// tag_1:
//   pop
//   pop
// tag_4:
//   jump	// out
// tag_3:
//     /* "":0:406   */
//   pop
//   pop
//   pop
