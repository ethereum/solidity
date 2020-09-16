{
	function f() -> z
	{
		z := 15
	}
	let x := 7
	let y := 8
	if eq(add(x, y), f()) { }
}
// ----
// step: reasoningBasedSimplifier
//
// {
//     function f() -> z
//     { z := 15 }
//     let x := 7
//     let y := 8
//     if eq(add(x, y), f()) { }
// }
