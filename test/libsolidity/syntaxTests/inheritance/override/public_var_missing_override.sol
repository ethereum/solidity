interface A {
    function foo() external returns (uint);
}
interface B {
    function foo() external returns (uint);
}
contract X is A, B {
	uint public override(A) foo;
}
// ----
// TypeError 4327: (154-165): Public state variable needs to specify overridden contract "B".
