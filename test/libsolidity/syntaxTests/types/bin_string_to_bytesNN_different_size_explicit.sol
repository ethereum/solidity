contract C {
   function f() public pure {
     bytes1 b1 = bytes1(bin"");
     bytes1 b2 = bytes1(bin"0001001000110100");
     bytes2 b3 = bytes2(bin"00010010");
     bytes2 b4 = bytes2(bin"0001001000110100");
     bytes2 b5 = bytes2(bin"000100100011010001010110");
     bytes3 b6 = bytes3(bin"0001001000110100");
     bytes3 b7 = bytes3(bin"000100100011010001010110");
     bytes3 b8 = bytes3(bin"00010010001101000101011001111000");
     bytes4 b9 = bytes4(bin"000100100011010001010110");
     bytes4 b10 = bytes4(bin"00010010001101000101011001111000");
     bytes4 b11 = bytes4(bin"0001001000110100010101100111100010010000");
   }
}
// ----
// TypeError 9640: (92-121): Explicit type conversion not allowed from "literal_string hex"1234"" to "bytes1". Literal is larger than the type.
// TypeError 9640: (228-265): Explicit type conversion not allowed from "literal_string hex"123456"" to "bytes2". Literal is larger than the type.
// TypeError 9640: (388-433): Explicit type conversion not allowed from "literal_string hex"12345678"" to "bytes3". Literal is larger than the type.
// TypeError 9640: (574-627): Explicit type conversion not allowed from "literal_string hex"1234567890"" to "bytes4". Literal is larger than the type.
