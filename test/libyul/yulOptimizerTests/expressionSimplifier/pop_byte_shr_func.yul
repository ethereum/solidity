{
  function f() -> x { mstore(0, 1337) }
  mstore(0, byte(0, shr(0x8, f())))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     function f() -> x
//     { mstore(x, 1337) }
//     pop(f())
//     mstore(0, 0)
// }
