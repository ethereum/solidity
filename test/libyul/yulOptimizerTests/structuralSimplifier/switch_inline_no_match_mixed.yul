{
	// Used to cause assert error
	let y := 200
	switch 3
	case "" { y := 8 }
	case 1 { y := 9 }
}
// ----
// structuralSimplifier
// {
//     let y := 200
// }
