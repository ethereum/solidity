contract C {
   function f() public pure {
     bytes1 b1 = bytes1(1);
     bytes2 b2 = bytes2(1);
     bytes2 b3 = bytes2(256);
     bytes3 b4 = bytes3(1);
     bytes3 b5 = bytes3(65536);
     bytes4 b6 = bytes4(1);
     bytes4 b7 = bytes4(16777216);
     bytes16 b8 = bytes16(1);
     bytes32 b9 = bytes32(1);
   }
}
// ----
// TypeError: (60-69): Explicit type conversion not allowed from "int_const 1" to "bytes1".
// TypeError: (88-97): Explicit type conversion not allowed from "int_const 1" to "bytes2".
// TypeError: (116-127): Explicit type conversion not allowed from "int_const 256" to "bytes2".
// TypeError: (146-155): Explicit type conversion not allowed from "int_const 1" to "bytes3".
// TypeError: (174-187): Explicit type conversion not allowed from "int_const 65536" to "bytes3".
// TypeError: (206-215): Explicit type conversion not allowed from "int_const 1" to "bytes4".
// TypeError: (234-250): Explicit type conversion not allowed from "int_const 16777216" to "bytes4".
// TypeError: (270-280): Explicit type conversion not allowed from "int_const 1" to "bytes16".
// TypeError: (300-310): Explicit type conversion not allowed from "int_const 1" to "bytes32".
