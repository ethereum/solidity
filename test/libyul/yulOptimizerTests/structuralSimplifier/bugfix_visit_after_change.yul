{
	// This tests that a bug is fixed where x := 1 was wrongfully
	// taken into account before actually visiting the if statement.
	let x := 0
	if x {
		x := 1
	}
}
// ====
// step: structuralSimplifier
// ----
// { let x := 0 }
