{
	let y := 200
	switch y
	case 1 { y := 9 }
	default { }
}
// ----
// structuralSimplifier
// {
//     let y := 200
//     switch y
//     case 1 {
//         y := 9
//     }
// }
