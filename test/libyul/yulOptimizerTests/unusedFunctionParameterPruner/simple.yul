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
//     sstore(f_1(1), 0)
//     function f() -> y
//     {
//         let w := mload(1)
//         y := mload(w)
//     }
//     function f_1(x_2) -> y_3
//     { y_3 := f() }
// }
