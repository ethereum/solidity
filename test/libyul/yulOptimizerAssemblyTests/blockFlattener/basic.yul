{
    {
        let _1 := mload(0)
        let f_a := mload(1)
        let f_r
        {
            f_a := mload(f_a)
            f_r := add(f_a, calldatasize())
        }
        let z := mload(2)
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
//         f_a := mload(f_a)
//         f_r := add(f_a, calldatasize())
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
//     /* "":114:117   */
//   dup2
//     /* "":108:118   */
//   mload
//     /* "":101:118   */
//   swap2
//   pop
//     /* "":147:161   */
//   calldatasize
//     /* "":142:145   */
//   dup3
//     /* "":138:162   */
//   add
//     /* "":131:162   */
//   swap1
//   pop
//     /* "":196:197   */
//   0x02
//     /* "":190:198   */
//   mload
//     /* "":6:204   */
//   pop
//   pop
//   pop
//   pop
