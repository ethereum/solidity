{
	mstore(f(3), 0)
	function f(x) -> y {
		switch x
		case 0 { y := 8 }
		case 1 { y := 9 }
		default { y := 10 }
	}
}
// ----
// step: fullSuite
//
// { { } }
