// Even if the functions pass the equality check, they are not movable.
{
	function f() -> a { }
	let b := sub(f(), f())
}
// ----
// expressionSimplifier
// {
//     function f() -> a
//     {
//     }
//     let b := sub(f(), f())
// }
