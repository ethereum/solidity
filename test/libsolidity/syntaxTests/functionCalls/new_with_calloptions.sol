contract D { constructor() public payable {} }
contract C {
    function foo() pure internal {
		new D{salt:"abc", value:3};
		new D{salt:"abc"};
		new D{value:5+5};
		new D{salt:"aabbcc"};
    }
}
// ====
// EVMVersion: >=constantinople
// ----
