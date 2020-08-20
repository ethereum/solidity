{
    sstore(f(1), 0)
    function f(x) -> y
    {
        let w := mload(1)
        y := mload(w)
    }
}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     sstore(f_6(1), 0)
//     function f() -> y
//     { y := mload(mload(1)) }
//     function f_6(x_7) -> y_8
//     { y_8 := f() }
// }
