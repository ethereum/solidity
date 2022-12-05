{
    pop(addmod(addmod(0x80000000000000000000000000000000000, 0xCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC, 0x800000000000000000000000000000000000), 0xCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC, 0x8000000000000000000000000000000000000))
}
// ====
// stackOptimization: true
// ----
//     /* "":240:279   */
//   0x08000000000000000000000000000000000000
//     /* "":172:238   */
//   0xcccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
//     /* "":131:169   */
//   0x800000000000000000000000000000000000
//     /* "":17:170   */
//   dup2
//     /* "":24:61   */
//   0x080000000000000000000000000000000000
//     /* "":17:170   */
//   addmod
//     /* "":10:280   */
//   addmod
//     /* "":6:281   */
//   pop
//     /* "":0:283   */
//   stop
