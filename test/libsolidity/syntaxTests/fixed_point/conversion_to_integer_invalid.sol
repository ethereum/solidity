uint constant x = uint(fixed8x1(0));
uint248 constant y = uint248(ufixed256x1(0));
// ----
// TypeError 9640: (18-35): Explicit type conversion not allowed from "fixed8x1" to "uint256".
// TypeError 9640: (58-81): Explicit type conversion not allowed from "ufixed256x1" to "uint248".
