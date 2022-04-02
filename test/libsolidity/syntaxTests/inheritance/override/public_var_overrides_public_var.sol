contract A {
    uint public foo;
}
contract X is A {
	uint public override foo;
}
// ----
// DeclarationError 9097: (55-79='uint public override foo'): Identifier already declared.
// TypeError 1452: (17-32='uint public foo'): Cannot override public state variable.
