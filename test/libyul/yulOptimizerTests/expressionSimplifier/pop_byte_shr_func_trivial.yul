{
  function f() -> x {}
  pop(byte(0, shr(0x8, f())))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     function f() -> x
//     { }
//     pop(byte(0, shr(0x8, f())))
// }
