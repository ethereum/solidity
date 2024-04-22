contract C {
   function f() public pure {
     bytes1 b1 = 0b1;
     bytes1 b2 = 0b100000000;
     bytes2 b3 = 0b11111111;
     bytes2 b4 = 0b100000000;
     bytes2 b5 = 0b10000000000000000;
     bytes3 b6 = 0b1111111111111111;
     bytes3 b7 = 0b10000000000000000;
     bytes3 b8 = 0b1000000000000000000000000;
     bytes4 b9 = 0b111111111111111111111111;
     bytes4 b10 = 0b1000000000000000000000000;
     bytes4 b11 = 0b100000000000000000000000000000000;
     bytes16 b12 = 0b1;
     bytes32 b13 = 0b1;
   }
}
// ----
// TypeError 9574: (48-63): Type int_const 1 is not implicitly convertible to expected type bytes1.
// TypeError 9574: (70-93): Type int_const 256 is not implicitly convertible to expected type bytes1.
// TypeError 9574: (100-122): Type int_const 255 is not implicitly convertible to expected type bytes2.
// TypeError 9574: (129-152): Type int_const 256 is not implicitly convertible to expected type bytes2.
// TypeError 9574: (159-190): Type int_const 65536 is not implicitly convertible to expected type bytes2.
// TypeError 9574: (197-227): Type int_const 65535 is not implicitly convertible to expected type bytes3.
// TypeError 9574: (234-265): Type int_const 65536 is not implicitly convertible to expected type bytes3.
// TypeError 9574: (272-311): Type int_const 16777216 is not implicitly convertible to expected type bytes3.
// TypeError 9574: (318-356): Type int_const 16777215 is not implicitly convertible to expected type bytes4.
// TypeError 9574: (363-403): Type int_const 16777216 is not implicitly convertible to expected type bytes4.
// TypeError 9574: (410-458): Type int_const 4294967296 is not implicitly convertible to expected type bytes4.
// TypeError 9574: (465-482): Type int_const 1 is not implicitly convertible to expected type bytes16.
// TypeError 9574: (489-506): Type int_const 1 is not implicitly convertible to expected type bytes32.
