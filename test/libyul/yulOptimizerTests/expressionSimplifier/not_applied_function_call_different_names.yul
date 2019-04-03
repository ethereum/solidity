{
	function f1() -> a { }
	function f2() -> b { }
	let c := sub(f1(), f2())
}
// ====
// step: expressionSimplifier
// ----
// {
//     function f1() -> a
//     {
//     }
//     function f2() -> b
//     {
//     }
//     let c := sub(f1(), f2())
// }
