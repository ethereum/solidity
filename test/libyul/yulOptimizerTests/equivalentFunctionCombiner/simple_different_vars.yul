{
  pop(f())
  pop(g())
  function f() -> b { let a := mload(0) b := a }
  function g() -> a { let b := mload(0) a := b }
}
// ----
// equivalentFunctionCombiner
// {
//     pop(f())
//     pop(f())
//     function f() -> b
//     {
//         let a := mload(0)
//         b := a
//     }
//     function g() -> a_1
//     {
//         let b_2 := mload(0)
//         a_1 := b_2
//     }
// }
