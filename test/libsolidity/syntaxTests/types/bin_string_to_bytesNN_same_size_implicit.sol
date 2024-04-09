contract C {
   function f() public pure {
     bytes1 b1 = bin"00000001";
     bytes1 b2 = bin"11111111";
     bytes2 b3 = bin"0000000100000000";
     bytes2 b4 = bin"1111111111111111";
     bytes3 b5 = bin"000000010000000000000000";
     bytes3 b6 = bin"111111111111111111111111";
     bytes4 b7 = bin"00000001000000000000000000000000";
     bytes4 b8 = bin"11111111111111111111111111111111";
     b1; b2; b3; b4; b5; b6; b7; b8;
   }
}
// ----
