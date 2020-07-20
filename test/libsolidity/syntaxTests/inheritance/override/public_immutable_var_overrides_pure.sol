abstract contract C {
    function foo() external pure virtual returns (uint);
}
contract X is C {
	uint public immutable override foo = 7;
}
// ----
// TypeError 6959: (100-138): Overriding public state variable changes state mutability from "pure" to "view".
