contract B { function f() public {} }
contract C is B { function f() payable public {} }
// ----
// TypeError: (56-86): Overriding function is missing 'override' specifier.
// TypeError: (56-86): Overriding function changes state mutability from "nonpayable" to "payable".
