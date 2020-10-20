// A test to see if the optimization step can deal with pop
// Expression splitter converts `pop(f(a))` into
// {
//   let z := f(a)
//   pop(z)
// }
// Unless `pop(z)` is removed, `f` cannot be rewritten.
{
    let a := sload(1)
    let z := f(a)
    pop(z)
    function f(x) -> y
    {
      sstore(x, x)
      y := sload(x)
    }
}
// ----
// step: unusedFunctionReturnParameterPruner
//
// {
//     let a := sload(1)
//     let z := f_1(a)
//     function f(x)
//     {
//         let y
//         sstore(x, x)
//         y := sload(x)
//     }
//     function f_1(x_2) -> y_3
//     { f(x_2) }
// }
