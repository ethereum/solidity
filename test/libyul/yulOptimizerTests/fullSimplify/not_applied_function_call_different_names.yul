{
	function f1() -> a { }
	function f2() -> b { }
	let c := sub(f1(), f2())
	mstore(0, c)
}
// ----
// step: fullSimplify
//
// {
//     mstore(0, sub(f1(), f2()))
//     function f1() -> a
//     { }
//     function f2() -> b
//     { }
// }
