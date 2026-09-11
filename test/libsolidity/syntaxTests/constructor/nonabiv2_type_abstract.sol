pragma abicoder v1;
abstract contract C {
	constructor(uint[][][] memory t) {}
}
// ====
// compileViaYul: false
// ----
// Warning 9511: (0-19): ABI coder v1 is deprecated and scheduled for removal. Use ABI coder v2 instead.
