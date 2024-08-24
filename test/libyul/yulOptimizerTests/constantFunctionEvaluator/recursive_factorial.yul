{
    function factorial(n) -> res {
        if lt(n, 2) {
            res := 1
            leave
        }
        res := mul(factorial(sub(n, 1)), n)
    }

    function f0() -> r
    { r := factorial(0) }
    function f1() -> r
    { r := factorial(1) }
    function f2() -> r
    { r := factorial(2) }
    function f3() -> r
    { r := factorial(3) }
    function f10() -> r
    { r := factorial(10) }
    function f20() -> r
    { r := factorial(20) }
    function f69() -> r
    { r := factorial(69) }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function factorial(n) -> res
//     {
//         if lt(n, 2)
//         {
//             res := 1
//             leave
//         }
//         res := mul(factorial(sub(n, 1)), n)
//     }
//     function f0() -> r
//     { r := 1 }
//     function f1() -> r_1
//     { r_1 := 1 }
//     function f2() -> r_2
//     { r_2 := 2 }
//     function f3() -> r_3
//     { r_3 := 6 }
//     function f10() -> r_4
//     { r_4 := 3628800 }
//     function f20() -> r_5
//     { r_5 := 2432902008176640000 }
//     function f69() -> r_6
//     { r_6 := factorial(69) }
// }
