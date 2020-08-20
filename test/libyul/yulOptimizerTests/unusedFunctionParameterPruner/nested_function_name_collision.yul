// Test case where the name `g` occurs at two different places. Test to see if name-collision can
// cause issues.
{
    sstore(h(1), 0)
    sstore(f(1), 0)

    function f(c) -> u
    {
        u := g(c)
        function g(d) -> w
        {
            w := 13
        }
    }

    function h(c) -> u
    {
        u := g(c)
        function g(d) -> w
        {
            w := 13
        }
    }

}
// ----
// step: unusedFunctionParameterPruner
//
// {
//     sstore(h(1), 0)
//     sstore(f(1), 0)
//     function g() -> w
//     {
//         let w_2 := 13
//         w := 13
//     }
//     function g_11(d_13) -> w_14
//     { w_14 := g() }
//     function f(c) -> u
//     { u := g_11(c) }
//     function g_3() -> w_5
//     {
//         let w_5_4 := 13
//         w_5 := 13
//     }
//     function g_3_12(d_4_15) -> w_5_16
//     { w_5_16 := g_3() }
//     function h(c_1) -> u_2
//     { u_2 := g_3_12(c_1) }
// }
