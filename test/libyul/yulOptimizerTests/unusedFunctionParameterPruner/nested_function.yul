// Test case to see if the step applies for nested functions. The function `j` has an unused argument.
{
    sstore(f(1), 0)
    sstore(h(1), 0)

    function f(a) -> x
    {
        x := g(1)
        x := add(x, 1)
        function g(b) -> y
        {
            b := add(b, 1)
            y := mload(b)
        }
    }

    function h(c) -> u
    {
        u := j(c)
        // d is unused
        function j(d) -> w
        {
            w := 13
            w := add(w, 1)
        }
    }

}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     sstore(f_1(1), 0)
//     sstore(h(1), 0)
//     function g(b) -> y
//     {
//         b := add(b, 1)
//         y := mload(b)
//     }
//     function f() -> x
//     {
//         x := g(1)
//         x := add(x, 1)
//     }
//     function f_1(a_3) -> x_4
//     { x_4 := f() }
//     function j() -> w
//     {
//         w := 13
//         w := add(13, 1)
//     }
//     function j_2(d_5) -> w_6
//     { w_6 := j() }
//     function h(c) -> u
//     { u := j_2(c) }
// }
