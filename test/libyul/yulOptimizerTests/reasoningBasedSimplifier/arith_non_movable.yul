{
	function f() -> z
	{
		sstore(1, 15)
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
//     {
//         sstore(1, 15)
//         z := 15
//     }
//     let x := 7
//     let y := 8
//     if eq(add(x, y), f()) { }
// }
