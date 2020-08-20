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
//     { w := 13 }
//     function g_1(d_3) -> w_4
//     { w_4 := g() }
//     function f(c) -> u
//     { u := g_1(c) }
//     function g_3() -> w_5
//     { w_5 := 13 }
//     function g_3_2(d_4_5) -> w_5_6
//     { w_5_6 := g_3() }
//     function h(c_1) -> u_2
//     { u_2 := g_3_2(c_1) }
// }
