contract C {
   function f() public pure {
     bytes1 b1 = 0x01;
     bytes1 b2 = 0xFF;
     bytes2 b3 = 0x0100;
     bytes2 b4 = 0xFFFF;
     bytes3 b5 = 0x010000;
     bytes3 b6 = 0xFFFFFF;
     bytes4 b7 = 0x01000000;
     bytes4 b8 = 0xFFFFFFFF;
     b1; b2; b3; b4; b5; b6; b7; b8;
   }
}
