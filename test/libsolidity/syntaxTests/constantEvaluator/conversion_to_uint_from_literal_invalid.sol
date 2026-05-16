contract A layout at uint(2**256) {}
contract B layout at uint(-1) {}
contract C layout at uint(3/2) {}
contract D layout at uint(3.14) {}
contract E layout at uint(1.1618e2) {}
// ----
// TypeError 9640: (21-33): Explicit type conversion not allowed from "int_const 1157...(70 digits omitted)...9936" to "uint256".
// TypeError 9640: (58-66): Explicit type conversion not allowed from "int_const -1" to "uint256".
// TypeError 9640: (91-100): Explicit type conversion not allowed from "rational_const 3 / 2" to "uint256".
// TypeError 9640: (125-135): Explicit type conversion not allowed from "rational_const 157 / 50" to "uint256".
// TypeError 9640: (160-174): Explicit type conversion not allowed from "rational_const 5809 / 50" to "uint256".
