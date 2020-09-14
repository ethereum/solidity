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
//     function f() -> a
//     { mstore(a, 1) }
//     sstore(sub(f(), f()), 8)
// }
