{
	if mstore(1, 1) {}
}
// ====
// dialect: evm
// ----
// TypeError 3950: (6-18): Expected expression to evaluate to one value, but got 0 values instead.
