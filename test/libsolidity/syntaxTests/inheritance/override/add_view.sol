contract B { function f() public {} }
contract C is B { function f() public view {} }
// ----
// TypeError: (56-83): Overriding function is missing 'override' specifier.
// TypeError: (56-83): Overriding function changes state mutability from "nonpayable" to "view".
