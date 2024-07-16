// SPDX-License-Identifier: GPL-3.0
pragma solidity >0.0.0;

contract B { constructor() payable { assembly { revert(0,0) } } }
contract A { constructor() payable { assembly { revert(0,0) } } }
contract C {
    // The subobject order should be determined by reference, not AST ID,
    // So the subobject for A should precede the subobject for B.
	A a = new A();
	B b = new B();
	fallback() external payable {
		assembly { revert(0,0) }
	}
}
