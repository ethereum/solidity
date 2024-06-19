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
//     function f_1(x) -> y_1
//     { y_1 := f() }
// }
