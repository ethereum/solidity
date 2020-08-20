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
//     sstore(f_20(1), 0)
//     sstore(h(1), 0)
//     function g(b) -> y
//     {
//         let b_3 := add(b, 1)
//         b := b_3
//         y := mload(b_3)
//     }
//     function f() -> x
//     {
//         let x_1 := g(1)
//         x := x_1
//         x := add(x_1, 1)
//     }
//     function f_20(a_22) -> x_23
//     { x_23 := f() }
//     function j() -> w
//     {
//         let w_6 := 13
//         w := 13
//         w := add(13, 1)
//     }
//     function j_21(d_24) -> w_25
//     { w_25 := j() }
//     function h(c) -> u
//     { u := j_21(c) }
// }
