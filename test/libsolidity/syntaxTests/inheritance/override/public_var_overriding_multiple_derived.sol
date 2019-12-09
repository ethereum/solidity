interface A {
    function foo() external returns (uint);
}
interface B {
    function foo() external returns (uint);
}
contract X is A, B {
	uint public override(A, B) foo;
}
contract Y is X {
}
// ----
