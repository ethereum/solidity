contract B { function f() virtual public {} }
contract C is B { function f() payable public {} }
// ----
// TypeError 9456: (64-94): Overriding function is missing "override" specifier.
// TypeError 6959: (64-94): Overriding function changes state mutability from "nonpayable" to "payable".
