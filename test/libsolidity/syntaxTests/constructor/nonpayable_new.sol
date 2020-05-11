contract A1 { constructor() public {} }
contract B1 is A1 {}

contract A2 { constructor() public payable {} }
contract B2 is A2 {}

contract B3 {}

contract B4 { constructor() public {} }

contract C {
	function f() public payable {
		new B1{value: 10}();
		new B2{value: 10}();
		new B3{value: 10}();
		new B4{value: 10}();
	}
}
// ----
// TypeError: (235-252): Cannot set option "value", since the constructor of contract B1 is not payable.
// TypeError: (258-275): Cannot set option "value", since the constructor of contract B2 is not payable.
// TypeError: (281-298): Cannot set option "value", since the constructor of contract B3 is not payable.
// TypeError: (304-321): Cannot set option "value", since the constructor of contract B4 is not payable.
