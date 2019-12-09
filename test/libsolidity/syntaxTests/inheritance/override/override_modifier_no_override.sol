abstract contract A {
}
abstract contract X is A {
	modifier f() override { _; }
}
// ----
// TypeError: (65-73): Modifier has override specified but does not override anything.
