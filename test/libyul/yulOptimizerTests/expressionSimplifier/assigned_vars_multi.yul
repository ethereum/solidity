{
	function f() -> x, z {}
	let c, d := f()
	let y := add(d, add(c, 7))
	sstore(y, 20)
}
// ----
// step: expressionSimplifier
//
// {
//     function f() -> x, z
//     { }
//     let c, d := f()
//     sstore(add(add(d, c), 7), 20)
// }
