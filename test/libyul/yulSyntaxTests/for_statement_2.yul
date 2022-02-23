{

	{ for {} 1 {} {} }
	{ for { let i := 1 } lt(i, 5) { i := add(i, 1) } {} }
	{ for {} 1 {} {} }
	{ let x := calldatasize() for { let i := 0} lt(i, x) { i := add(i, 1) } { mstore(i, 2) } }
}
// ====
// dialect: evm
// ----
