{
	function f() -> x, z {}
	let c, d := f()
	let y := add(d, add(c, 7))
}
// ====
// step: expressionSimplifier
// ----
// {
//     function f() -> x, z
//     {
//     }
//     let c, d := f()
//     let y := add(add(d, c), 7)
// }
