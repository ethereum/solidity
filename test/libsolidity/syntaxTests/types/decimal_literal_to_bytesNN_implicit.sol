contract C {
   function f() public pure {
     bytes1 b1 = 1;
     bytes2 b2 = 1;
     bytes2 b3 = 256;
     bytes3 b4 = 1;
     bytes3 b5 = 65536;
     bytes4 b6 = 1;
     bytes4 b7 = 16777216;
     bytes16 b8 = 1;
     bytes32 b9 = 1;
   }
}
// ----
// TypeError 9574: (48-61='bytes1 b1 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes1.
// TypeError 9574: (68-81='bytes2 b2 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes2.
// TypeError 9574: (88-103='bytes2 b3 = 256'): Type int_const 256 is not implicitly convertible to expected type bytes2.
// TypeError 9574: (110-123='bytes3 b4 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes3.
// TypeError 9574: (130-147='bytes3 b5 = 65536'): Type int_const 65536 is not implicitly convertible to expected type bytes3.
// TypeError 9574: (154-167='bytes4 b6 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes4.
// TypeError 9574: (174-194='bytes4 b7 = 16777216'): Type int_const 16777216 is not implicitly convertible to expected type bytes4.
// TypeError 9574: (201-215='bytes16 b8 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes16.
// TypeError 9574: (222-236='bytes32 b9 = 1'): Type int_const 1 is not implicitly convertible to expected type bytes32.
