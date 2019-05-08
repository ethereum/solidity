{
	function f(a, r) -> x { x := g(a, g(r, r)) }
	function g(b, s) -> y { y := f(b, f(s, s)) }
	let y := g(calldatasize(), 7)
}
// ====
// step: expressionInliner
// ----
// {
//     function f(a, r) -> x
//     { x := g(a, f(r, f(r, r))) }
//     function g(b, s) -> y
//     {
//         y := f(b, g(s, f(s, f(s, s))))
//     }
//     let y_1 := f(calldatasize(), g(7, f(7, f(7, 7))))
// }
