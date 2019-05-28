// Does not inline because mload could be moved out of sequence
{
    function f(a) -> x { x := a }
    let y := f(mload(2))
}
// ====
// step: expressionInliner
// ----
// {
//     function f(a) -> x
//     { x := a }
//     let y := f(mload(2))
// }
