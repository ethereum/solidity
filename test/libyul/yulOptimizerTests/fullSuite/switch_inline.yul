{
	mstore(f(1), 0)
	function f(x) -> y {
		switch x
		case 0 { y := 8 }
		case 1 { y := 9 }
	}
}
// ====
// step: fullSuite
// ----
// { { mstore(9, 0) } }
