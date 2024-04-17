contract C {
   function f() public pure {
     bytes1 b1 = hex"";
     bytes1 b2 = hex"1234";
     bytes2 b3 = hex"12";
     bytes2 b4 = hex"1234";
     bytes2 b5 = hex"123456";
     bytes3 b6 = hex"1234";
     bytes3 b7 = hex"123456";
     bytes3 b8 = hex"12345678";
     bytes4 b9 = hex"123456";
     bytes4 b10 = hex"12345678";
     bytes4 b11 = hex"1234567890";
   }
}
// ----
// TypeError 9574: (72-93): Type literal_string hex"1234" is not implicitly convertible to expected type bytes1. Literal is larger than the type.
// TypeError 9574: (154-177): Type literal_string hex"123456" is not implicitly convertible to expected type bytes2. Literal is larger than the type.
// TypeError 9574: (242-267): Type literal_string hex"12345678" is not implicitly convertible to expected type bytes3. Literal is larger than the type.
// TypeError 9574: (337-365): Type literal_string hex"1234567890" is not implicitly convertible to expected type bytes4. Literal is larger than the type.
