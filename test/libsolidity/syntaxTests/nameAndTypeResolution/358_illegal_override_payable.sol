contract B { function f() payable public {} }
contract C is B { function f() public {} }
// ----
// TypeError: (64-86): Overriding function is missing 'override' specifier.
// TypeError: (64-86): Overriding function changes state mutability from "payable" to "nonpayable".
