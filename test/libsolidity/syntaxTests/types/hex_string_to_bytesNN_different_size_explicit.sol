contract C {
   function f() public pure {
     bytes1 b1 = bytes1(hex"");
     bytes1 b2 = bytes1(hex"1234");
     bytes2 b3 = bytes2(hex"12");
     bytes2 b4 = bytes2(hex"1234");
     bytes2 b5 = bytes2(hex"123456");
     bytes3 b6 = bytes3(hex"1234");
     bytes3 b7 = bytes3(hex"123456");
     bytes3 b8 = bytes3(hex"12345678");
     bytes4 b9 = bytes4(hex"123456");
     bytes4 b10 = bytes4(hex"12345678");
     bytes4 b11 = bytes4(hex"1234567890");
   }
}
// ----
// TypeError 9640: (92-109): Explicit type conversion not allowed from "literal_string hex"1234"" to "bytes1". Literal is larger than the type.
// TypeError 9640: (198-217): Explicit type conversion not allowed from "literal_string hex"123456"" to "bytes2". Literal is larger than the type.
// TypeError 9640: (310-331): Explicit type conversion not allowed from "literal_string hex"12345678"" to "bytes3". Literal is larger than the type.
// TypeError 9640: (430-453): Explicit type conversion not allowed from "literal_string hex"1234567890"" to "bytes4". Literal is larger than the type.
