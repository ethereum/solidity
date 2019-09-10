{
	let y := 200
	switch 3
	case 0 { y := 8 }
	case 1 { y := 9 }
}
// ====
// step: structuralSimplifier
// ----
// { let y := 200 }
