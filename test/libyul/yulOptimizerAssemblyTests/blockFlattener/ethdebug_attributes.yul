{
    {
        let _1 := mload(0)
        let f_a := mload(1)
        let f_r
        {
            /// @debug.set {"scope":1}
            f_a := mload(f_a)
            f_r := add(f_a, calldatasize())
        }
        {
            /// @debug.set {"scope":2}
            let z := mload(2)
        }
    }
}
// ----
// step: blockFlattener
//
// {
//     {
//         let _1 := mload(0)
//         let f_a := mload(1)
//         let f_r
//         /// @debug.set {"scope":1}
//         f_a := mload(f_a)
//         f_r := add(f_a, calldatasize())
//         /// @debug.set {"scope":2}
//         let z := mload(2)
//     }
// }
//
// Assembly:
//     /* "":32:33   */
//   0x00
//     /* "":26:34   */
//   mload
//     /* "":60:61   */
//   0x01
//     /* "":54:62   */
//   mload
//     /* "":71:78   */
//   0x00
//     /* "":153:156   */
//   dup2  // @debug.set {"scope":1}
//     /* "":147:157   */
//   mload  // @debug.set {"scope":1}
//     /* "":140:157   */
//   swap2  // @debug.set {"scope":1}
//   pop  // @debug.set {"scope":1}
//     /* "":186:200   */
//   calldatasize  // @debug.set {"scope":1}
//     /* "":181:184   */
//   dup3  // @debug.set {"scope":1}
//     /* "":177:201   */
//   add  // @debug.set {"scope":1}
//     /* "":170:201   */
//   swap1  // @debug.set {"scope":1}
//   pop  // @debug.set {"scope":1}
//     /* "":288:289   */
//   0x02  // @debug.set {"scope":2}
//     /* "":282:290   */
//   mload  // @debug.set {"scope":2}
//     /* "":6:306   */
//   pop
//   pop
//   pop
//   pop
