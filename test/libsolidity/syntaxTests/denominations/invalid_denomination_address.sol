contract C {
	address a = 0x11111122222333334444455555666667777788888 wei;
}
// ----
// TypeError 5145: (26-73): Hexadecimal numbers cannot be used with unit denominations. You can use an expression of the form "0x1234 * 1 day" instead.
