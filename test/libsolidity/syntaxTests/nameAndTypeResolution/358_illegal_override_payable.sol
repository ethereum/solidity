contract B { function f() payable virtual public {} }
contract C is B { function f() public {} }
// ----
// TypeError: (72-94): Overriding function is missing 'override' specifier.
// TypeError: (72-94): Overriding function changes state mutability from "payable" to "nonpayable".
