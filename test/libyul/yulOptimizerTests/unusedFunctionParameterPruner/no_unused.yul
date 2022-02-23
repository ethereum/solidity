// A test where all parameters are used
{
    sstore(f(1), 0)
    function f(a) -> x { x := a }
    function g(b) -> y { y := g(b) }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     sstore(f(1), 0)
//     function f(a) -> x
//     { x := a }
//     function g(b) -> y
//     { y := g(b) }
// }
