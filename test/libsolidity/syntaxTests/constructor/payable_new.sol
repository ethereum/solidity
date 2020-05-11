contract A1 {}
contract B1 is A1 { constructor() public payable {} }

contract A2 { constructor() public {} }
contract B2 is A2 { constructor() public payable {} }

contract B3 { constructor() public payable {} }

contract C {
	function f() public payable {
		new B1{value: 10}();
		new B2{value: 10}();
		new B3{value: 10}();
	}
}
// ----
