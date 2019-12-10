abstract contract A {
	int public testvar;
}
abstract contract X is A {
	int public override testvar;
}
// ----
// DeclarationError: (73-100): Identifier already declared.
// TypeError: (84-92): Public state variable has override specified but does not override anything.
