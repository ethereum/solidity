contract B { function f() virtual public view {} }
contract C is B { function f() public {} }
// ----
// TypeError: (69-91): Overriding function is missing 'override' specifier.
// TypeError: (69-91): Overriding function changes state mutability from "view" to "nonpayable".
