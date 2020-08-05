contract C {
	fallback() external {
		assembly {
			let x := calldataload(0)
			x := x
			sstore(0, x)
		}
	}
}
