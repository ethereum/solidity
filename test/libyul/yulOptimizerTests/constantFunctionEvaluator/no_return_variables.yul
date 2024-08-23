{
    function should_be_empty() {
        pop(foo(1, 2))
    }

    function should_remain() {
        pop(bar(1, 2))
    }

    function foo(x, y) -> s {
        s := add(x, y)
    }

    function bar(x, y) -> s {
        s := mul(x, y)
        sstore(1, s)
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function should_be_empty()
//     { }
//     function should_remain()
//     { pop(bar(1, 2)) }
//     function foo(x, y) -> s
//     { s := add(x, y) }
//     function bar(x_1, y_2) -> s_3
//     {
//         s_3 := mul(x_1, y_2)
//         sstore(1, s_3)
//     }
// }
