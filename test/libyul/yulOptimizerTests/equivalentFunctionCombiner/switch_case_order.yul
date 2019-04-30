{
  f(0)
  g(1)
  function f(x) { switch x case 0 { mstore(0, 42) } case 1 { mstore(1, 42) } }
  function g(x) { switch x case 1 { mstore(1, 42) } case 0 { mstore(0, 42) } }
}
// ====
// step: equivalentFunctionCombiner
// ----
// {
//     f(0)
//     f(1)
//     function f(x)
//     {
//         switch x
//         case 0 {
//             mstore(0, 42)
//         }
//         case 1 {
//             mstore(1, 42)
//         }
//     }
//     function g(x_1)
//     {
//         switch x_1
//         case 1 {
//             mstore(1, 42)
//         }
//         case 0 {
//             mstore(0, 42)
//         }
//     }
// }
