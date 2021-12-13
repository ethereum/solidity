{
  sstore(0, byte(0, shr(0x8, call(0, 0, 0, 0, 0, 0, 0))))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0
//         pop(call(_1, _1, _1, _1, _1, _1, _1))
//         sstore(_1, 0)
//     }
// }
