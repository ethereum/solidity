{
  for {} div(create(0, 1, 0), shl(msize(), 1)) {}
  {
  }
}
// ====
// EVMVersion: >byzantium
// ----
// step: expressionSimplifier
//
// {
//     {
//         for { } div(create(0, 1, 0), shl(msize(), 1)) { }
//         { }
//     }
// }
