{
    {
	let a := 42
        sstore(0,mload(calldataload(add(a, 43))))
    }
}
// ====
// stackOptimization: true
// ----
//     /* "":64:66   */
//   0x2b
//     /* "":18:20   */
//   0x2a
//     /* "":57:67   */
//   add
//     /* "":44:68   */
//   calldataload
//     /* "":38:69   */
//   mload
//     /* "":36:37   */
//   0x00
//     /* "":29:70   */
//   sstore
//   stop
