contract C {
	uint constant x = 0b01 wei;
}
// ----
// TypeError 5146: (32-40): Binary numbers cannot be used with unit denominations. You can use an expression of the form "0b1011 * 1 days" instead.
