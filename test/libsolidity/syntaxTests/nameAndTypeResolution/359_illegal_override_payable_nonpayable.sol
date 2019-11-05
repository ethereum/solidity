contract B { function f() virtual public {} }
contract C is B { function f() payable public {} }
// ----
// TypeError: (64-94): Overriding function is missing 'override' specifier.
// TypeError: (64-94): Overriding function changes state mutability from "nonpayable" to "payable".
