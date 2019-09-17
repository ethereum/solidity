{
  for {} div(create(0, 1, 0), shl(msize(), 1)) {}
  {
  }
}
// ====
// step: expressionSimplifier
// EVMVersion: >byzantium
// ----
// {
//     for { } div(create(0, 1, 0), shl(msize(), 1)) { }
//     { }
// }
