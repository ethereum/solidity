// A test to see if UnusedFunctionReturnParameterPruner can deal with pop
// Expression splitter converts `pop(f(a))` into
// {
//   let z := f(a)
//   pop(z)
// }
// Unless `pop(z)` is removed, `f` cannot be rewritten.
{
    let a := sload(1)
    pop(f(a))
    function f(x) -> y
    {
      if iszero(calldataload(0)) { leave }
      sstore(x, x)
      y := sload(x)
    }
}
// ----
// step: fullSuite
//
// {
//     { pop(f_17(sload(1))) }
//     function f(x)
//     {
//         if iszero(calldataload(0)) { leave }
//         sstore(x, x)
//     }
//     function f_17(x) -> y
//     { f(x) }
// }
