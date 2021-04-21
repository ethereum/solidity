contract C {
   function f() public pure {
     bytes1 b1 = bytes1(hex"01");
     bytes1 b2 = bytes1(hex"FF");
     bytes2 b3 = bytes2(hex"0100");
     bytes2 b4 = bytes2(hex"FFFF");
     bytes3 b5 = bytes3(hex"010000");
     bytes3 b6 = bytes3(hex"FFFFFF");
     bytes4 b7 = bytes4(hex"01000000");
     bytes4 b8 = bytes4(hex"FFFFFFFF");
     b1; b2; b3; b4; b5; b6; b7; b8;
   }
}
// ----
