contract B { function f() view {} }
contract C is B { function f() public {} }
// ----
// TypeError: (54-76): Overriding function changes state mutability from "view" to "nonpayable".
