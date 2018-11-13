contract B { function f() public view {} }
contract C is B { function f() public {} }
// ----
// TypeError: (61-83): Overriding function changes state mutability from "view" to "nonpayable".
