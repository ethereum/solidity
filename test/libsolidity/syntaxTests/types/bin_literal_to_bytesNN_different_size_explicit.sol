contract C {
   function f() public pure {
     bytes1 b1 = bytes1(0b1);
     bytes1 b2 = bytes1(0b100000000);
     bytes2 b3 = bytes2(0b11111111);
     bytes2 b4 = bytes2(0b100000000);
     bytes2 b5 = bytes2(0b10000000000000000);
     bytes3 b6 = bytes3(0b1111111111111111);
     bytes3 b7 = bytes3(0b10000000000000000);
     bytes3 b8 = bytes3(0b1000000000000000000000000);
     bytes4 b9 = bytes4(0b111111111111111111111111);
     bytes4 b10 = bytes4(0b1000000000000000000000000);
     bytes4 b11 = bytes4(0b100000000000000000000000000000000);
     bytes16 b12 = bytes16(0b1);
     bytes32 b13 = bytes32(0b1);
   }
}
// ----
// TypeError 9640: (60-71): Explicit type conversion not allowed from "int_const 1" to "bytes1".
// TypeError 9640: (90-103): Explicit type conversion not allowed from "int_const 256" to "bytes1".
// TypeError 9640: (122-134): Explicit type conversion not allowed from "int_const 255" to "bytes2".
// TypeError 9640: (153-166): Explicit type conversion not allowed from "int_const 256" to "bytes2".
// TypeError 9640: (185-200): Explicit type conversion not allowed from "int_const 65536" to "bytes2".
// TypeError 9640: (219-233): Explicit type conversion not allowed from "int_const 65535" to "bytes3".
// TypeError 9640: (252-267): Explicit type conversion not allowed from "int_const 65536" to "bytes3".
// TypeError 9640: (286-303): Explicit type conversion not allowed from "int_const 16777216" to "bytes3".
// TypeError 9640: (322-338): Explicit type conversion not allowed from "int_const 16777215" to "bytes4".
// TypeError 9640: (358-375): Explicit type conversion not allowed from "int_const 16777216" to "bytes4".
// TypeError 9640: (395-414): Explicit type conversion not allowed from "int_const 4294967296" to "bytes4".
// TypeError 9640: (435-447): Explicit type conversion not allowed from "int_const 1" to "bytes16".
// TypeError 9640: (468-480): Explicit type conversion not allowed from "int_const 1" to "bytes32".
