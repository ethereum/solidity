{
  sstore(0, byte(0, shr(0x8, call(0, 0, 0, 0, 0, 0, 0))))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         pop(call(0, 0, 0, 0, 0, 0, 0))
//         sstore(0, 0)
//     }
// }
