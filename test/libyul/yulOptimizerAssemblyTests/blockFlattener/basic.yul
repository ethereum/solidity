/// @debug.set {"block": 0}
{
    /// @debug.set {"block": 1}
    {
        let _1 := mload(0)
        let f_a := mload(1)
        let f_r
        /// @debug.set {"block": 2}
        {
            f_a := mload(f_a)
            f_r := add(f_a, calldatasize())
        }
        /// @debug.set {"block": 3}
        let z := mload(/** @debug.set {"block": 4} **/2/** @debug.set {"block": 5} **/)
    }
}
// ----
// step: blockFlattener
//
// /// @debug.set {"block":0}
// {
//     /// @debug.set {"block":1}
//     {
//         let _1 := mload(0)
//         let f_a := mload(1)
//         let f_r
//         /// @debug.set {"block":2}
//         f_a := mload(f_a)
//         f_r := add(f_a, calldatasize())
//         /// @debug.set {"block":3}
//         let z := mload(/** @debug.set {"block":4} */ 2)
//     }
// }
//
// Assembly:
//     /* "":92:93   */
//   0x00   // @debug.set {"block":1}
//     /* "":86:94   */
//   mload   // @debug.set {"block":1}
//     /* "":120:121   */
//   0x01   // @debug.set {"block":1}
//     /* "":114:122   */
//   mload   // @debug.set {"block":1}
//     /* "":131:138   */
//   0x00   // @debug.set {"block":1}
//     /* "":210:213   */
//   dup2   // @debug.set {"block":2}
//     /* "":204:214   */
//   mload   // @debug.set {"block":2}
//     /* "":197:214   */
//   swap2   // @debug.set {"block":2}
//   pop   // @debug.set {"block":2}
//     /* "":243:257   */
//   calldatasize   // @debug.set {"block":2}
//     /* "":238:241   */
//   dup3   // @debug.set {"block":2}
//     /* "":234:258   */
//   add   // @debug.set {"block":2}
//     /* "":227:258   */
//   swap1   // @debug.set {"block":2}
//   pop   // @debug.set {"block":2}
//     /* "":359:360   */
//   0x02   // @debug.set {"block":4}
//     /* "":322:392   */
//   mload   // @debug.set {"block":3}
//     /* "":66:398   */
//   pop   // @debug.set {"block":1}
//   pop   // @debug.set {"block":1}
//   pop   // @debug.set {"block":1}
//   pop   // @debug.set {"block":1}
