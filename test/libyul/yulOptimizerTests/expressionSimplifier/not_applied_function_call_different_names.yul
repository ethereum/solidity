{
	function f1() -> a { }
	function f2() -> b { }
	let c := sub(f1(), f2())
	sstore(0, c)
}
// ----
// step: expressionSimplifier
//
// {
//     sstore(0, sub(f1(), f2()))
//     function f1() -> a
//     { }
//     function f2() -> b
//     { }
// }
