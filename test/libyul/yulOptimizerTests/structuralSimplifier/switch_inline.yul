{
	let y := 200
	switch 1
	case 0 { y := 8 }
	case 1 { y := 9 }
}
// ----
// structuralSimplifier
// {
//     let y := 200
//     {
//         y := 9
//     }
// }
