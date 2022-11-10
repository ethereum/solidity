{
  f()
  g()
  function f() { mstore(0x01, mload(0x00)) }
  function g() { mstore(1, mload(0)) }
}
// ----
// step: equivalentFunctionCombiner
//
// {
//     f()
//     f()
//     function f()
//     { mstore(0x01, mload(0x00)) }
//     function g()
//     { mstore(1, mload(0)) }
// }
