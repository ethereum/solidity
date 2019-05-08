{
	let y := 200
	switch calldataload(0)
	case 0 { }
	case 1 { y := 9 }
	case 2 { y := 10 }
}
// ====
// step: structuralSimplifier
// ----
// {
//     let y := 200
//     switch calldataload(0)
//     case 1 { y := 9 }
//     case 2 { y := 10 }
// }
