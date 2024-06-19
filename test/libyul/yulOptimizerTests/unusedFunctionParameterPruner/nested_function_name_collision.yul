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
            sstore(0, w)
        }
    }

    function h(c) -> u
    {
        u := g(c)
        function g(d) -> w
        {
            w := 13
            sstore(0, w)
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
//         w := 13
//         sstore(0, 13)
//     }
//     function g_2(d) -> w_2
//     { w_2 := g() }
//     function f(c) -> u
//     { u := g_2(c) }
//     function g_1() -> w_1
//     {
//         w_1 := 13
//         sstore(0, 13)
//     }
//     function g_3(d_1) -> w_3
//     { w_3 := g_1() }
//     function h(c_1) -> u_1
//     { u_1 := g_3(c_1) }
// }
