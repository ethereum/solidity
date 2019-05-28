// Even if the functions pass the equality check, they are not movable.
{
	function f() -> a { }
	let b := sub(f(), f())
	mstore(0, b)
}
// ====
// step: fullSimplify
// ----
// {
//     function f() -> a
//     { }
//     mstore(0, sub(f(), f()))
// }
