{
  f()
  g()
  function f() { mstore(1, mload(0)) }
  function g() { mstore(1, mload(0)) }
}
// ----
// equivalentFunctionCombiner
// {
//     f()
//     f()
//     function f()
//     {
//         mstore(1, mload(0))
//     }
//     function g()
//     {
//         mstore(1, mload(0))
//     }
// }
