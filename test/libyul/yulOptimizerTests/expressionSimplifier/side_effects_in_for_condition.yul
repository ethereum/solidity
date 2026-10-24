{
  for {} div(create(0, 1, 1), shl(msize(), 1)) {}
  {
  }
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         for { } div(create(0, 1, 1), shl(msize(), 1)) { }
//         { }
//     }
// }
