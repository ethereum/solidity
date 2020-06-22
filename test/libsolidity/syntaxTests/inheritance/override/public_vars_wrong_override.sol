contract X {
	uint public override foo;
}
// ----
// TypeError 7792: (26-34): Public state variable has override specified but does not override anything.
