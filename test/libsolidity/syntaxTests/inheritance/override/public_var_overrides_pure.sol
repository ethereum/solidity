abstract contract C {
    function foo() external pure virtual returns (uint);
}
contract X is C {
	uint public override foo;
}
// ----
// TypeError 6959: (100-124): Overriding public state variable changes state mutability from "pure" to "view".
