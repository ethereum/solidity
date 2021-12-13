{
    let a
    function f() -> x { x := g() }
    function g() -> y { y := f() }
    function h() -> z { z := g() }
    a := h()
}
// ----
// step: circularReferencesPruner
//
// {
//     {
//         let a
//         a := h()
//     }
//     function f() -> x
//     { x := g() }
//     function g() -> y
//     { y := f() }
//     function h() -> z
//     { z := g() }
// }
