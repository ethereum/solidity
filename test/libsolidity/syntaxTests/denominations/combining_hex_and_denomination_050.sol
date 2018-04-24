pragma experimental "v0.5.0";
contract C {
	uint constant x = 0x01 wei;
}
// ----
// TypeError: (62-70): Hexadecimal numbers cannot be used with unit denominations. You can use an expression of the form "0x1234 * 1 day" instead.
