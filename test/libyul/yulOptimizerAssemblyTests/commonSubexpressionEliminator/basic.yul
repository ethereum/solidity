{
    let a := /** @debug.set {"assignment":"a"} */ mul(1, codesize()) /** @debug.set {} */
    let b := /** @debug.set {"assignment":"b"} */ mul(1, codesize()) /** @debug.set {} */
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let a := /** @debug.set {"assignment":"a"} */ mul(1, codesize())
//     let b := a
// }
//
// Assembly:
//     /* "":59:69   */
//   codesize   // @debug.set {"assignment":"a"}
//     /* "":56:57   */
//   0x01   // @debug.set {"assignment":"a"}
//     /* "":52:70   */
//   mul   // @debug.set {"assignment":"a"}
//     /* "":142:160   */
//   dup1   // @debug.set [{"assignment":"a"},{"assignment":"b"}]
//     /* "":0:183   */
//   pop
//   pop
