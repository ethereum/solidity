{
	switch mstore(1, 1)
	case 1 {}
	default {}
}
// ====
// dialect: evm
// ----
// TypeError 3950: (10-22): Expected expression to evaluate to one value, but got 0 values instead.
