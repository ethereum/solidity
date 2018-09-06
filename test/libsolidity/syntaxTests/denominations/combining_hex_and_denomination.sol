contract C {
	uint constant x = 0x01 wei;
}
// ----
// TypeError: (32-40): Hexadecimal numbers cannot be used with unit denominations. You can use an expression of the form "0x1234 * 1 day" instead.
