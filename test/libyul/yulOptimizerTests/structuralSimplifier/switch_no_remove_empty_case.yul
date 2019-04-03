{
	let y := 200
	switch calldataload(0)
	case 0 { }
	case 1 { y := 9 }
	default { y := 100 }
}
// ====
// step: structuralSimplifier
// ----
// {
//     let y := 200
//     switch calldataload(0)
//     case 0 {
//     }
//     case 1 {
//         y := 9
//     }
//     default {
//         y := 100
//     }
// }
