contract B { function f() public {} }
contract C is B { function f() view {} }
// ----
// TypeError: (56-76): Overriding function changes state mutability from "nonpayable" to "view".
