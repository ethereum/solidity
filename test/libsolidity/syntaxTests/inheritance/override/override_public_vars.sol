abstract contract A {
	int public testvar;
}
abstract contract X is A {
	int public override testvar;
}
// ----
