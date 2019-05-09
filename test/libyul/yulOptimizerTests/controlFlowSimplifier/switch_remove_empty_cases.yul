{
	let y := 200
	switch calldataload(0)
	case 0 { }
	case 1 { y := 9 }
	default { }
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     let y := 200
//     if eq(1, calldataload(0)) { y := 9 }
// }
