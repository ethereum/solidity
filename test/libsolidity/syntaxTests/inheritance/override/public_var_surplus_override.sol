interface A {
    function foo() external returns (uint);
}
interface B {}
contract X is A {
	uint public override(A, B) foo;
}
// ----
// TypeError 2353: (106-120): Invalid contract specified in override list: "B".
