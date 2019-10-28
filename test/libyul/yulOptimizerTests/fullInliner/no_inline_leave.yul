{
    function g() -> x { x := 8 leave }
	function f(a) { a := g() }
    let a1 := calldataload(0)
    f(a1)
}
// ====
// step: fullInliner
// ----
// {
//     {
//         let a_2 := calldataload(0)
//         a_2 := g()
//     }
//     function g() -> x
//     {
//         x := 8
//         leave
//     }
//     function f(a)
//     { a := g() }
// }
