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
//     let c, d := f()
//     sstore(add(add(d, c), 7), 20)
//     function f() -> x, z
//     { }
// }
