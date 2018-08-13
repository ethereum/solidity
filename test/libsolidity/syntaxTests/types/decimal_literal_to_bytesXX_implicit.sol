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
// TypeError: (48-61): Type int_const 1 is not implicitly convertible to expected type bytes1.
// TypeError: (68-81): Type int_const 1 is not implicitly convertible to expected type bytes2.
// TypeError: (88-103): Type int_const 256 is not implicitly convertible to expected type bytes2.
// TypeError: (110-123): Type int_const 1 is not implicitly convertible to expected type bytes3.
// TypeError: (130-147): Type int_const 65536 is not implicitly convertible to expected type bytes3.
// TypeError: (154-167): Type int_const 1 is not implicitly convertible to expected type bytes4.
// TypeError: (174-194): Type int_const 16777216 is not implicitly convertible to expected type bytes4.
// TypeError: (201-215): Type int_const 1 is not implicitly convertible to expected type bytes16.
// TypeError: (222-236): Type int_const 1 is not implicitly convertible to expected type bytes32.
