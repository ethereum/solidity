{
    function f() -> x { x := 7 leave }
    function g() -> x { leave x := 7 }
    function h() -> x { if x { leave } }
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     function f() -> x
//     { x := 7 }
//     function g() -> x_1
//     {
//         leave
//         x_1 := 7
//     }
//     function h() -> x_2
//     { if x_2 { leave } }
// }
