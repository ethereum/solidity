// Even if the functions pass the equality check, they are not movable.
{
	function f() -> a { mstore(0, 1) }
	let b := sub(f(), f())
	sstore(b, 8)
}
// ----
// step: expressionSimplifier
//
// {
//     { sstore(sub(f(), f()), 8) }
//     function f() -> a
//     { mstore(0, 1) }
// }
