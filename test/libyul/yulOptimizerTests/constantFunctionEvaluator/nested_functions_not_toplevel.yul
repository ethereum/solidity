{
    function foo(x) -> ret
    {
        ret := div(x, bar())

        function bar() -> k
        { k := exp(2, 10) }
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function foo(x) -> ret
//     {
//         ret := div(x, bar())
//         function bar() -> k
//         { k := 1024 }
//     }
// }
