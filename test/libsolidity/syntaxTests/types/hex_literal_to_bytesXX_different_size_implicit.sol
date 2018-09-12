contract C {
   function f() public pure {
     bytes1 b1 = 0x1;
     bytes1 b2 = 0x100;
     bytes2 b3 = 0xFF;
     bytes2 b4 = 0x100;
     bytes2 b5 = 0x10000;
     bytes3 b6 = 0xFFFF;
     bytes3 b7 = 0x10000;
     bytes3 b8 = 0x1000000;
     bytes4 b9 = 0xFFFFFF;
     bytes4 b10 = 0x1000000;
     bytes4 b11 = 0x100000000;
     bytes16 b12 = 0x1;
     bytes32 b13 = 0x1;
   }
}
// ----
// TypeError: (48-63): Type int_const 1 is not implicitly convertible to expected type bytes1.
// TypeError: (70-87): Type int_const 256 is not implicitly convertible to expected type bytes1.
// TypeError: (94-110): Type int_const 255 is not implicitly convertible to expected type bytes2.
// TypeError: (117-134): Type int_const 256 is not implicitly convertible to expected type bytes2.
// TypeError: (141-160): Type int_const 65536 is not implicitly convertible to expected type bytes2.
// TypeError: (167-185): Type int_const 65535 is not implicitly convertible to expected type bytes3.
// TypeError: (192-211): Type int_const 65536 is not implicitly convertible to expected type bytes3.
// TypeError: (218-239): Type int_const 16777216 is not implicitly convertible to expected type bytes3.
// TypeError: (246-266): Type int_const 16777215 is not implicitly convertible to expected type bytes4.
// TypeError: (273-295): Type int_const 16777216 is not implicitly convertible to expected type bytes4.
// TypeError: (302-326): Type int_const 4294967296 is not implicitly convertible to expected type bytes4.
// TypeError: (333-350): Type int_const 1 is not implicitly convertible to expected type bytes16.
// TypeError: (357-374): Type int_const 1 is not implicitly convertible to expected type bytes32.
