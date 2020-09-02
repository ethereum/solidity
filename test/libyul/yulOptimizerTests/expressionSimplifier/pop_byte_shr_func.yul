{
  function f() -> x { mstore(0, 1337) }
  pop(byte(0, shr(0x8, f())))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     function f() -> x
//     { mstore(0, 1337) }
//     pop(byte(0, shr(0x8, f())))
// }
