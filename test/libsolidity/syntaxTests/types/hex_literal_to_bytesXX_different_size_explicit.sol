contract C {
   function f() public pure {
     bytes1 b1 = bytes1(0x1);
     bytes1 b2 = bytes1(0x100);
     bytes2 b3 = bytes2(0xFF);
     bytes2 b4 = bytes2(0x100);
     bytes2 b5 = bytes2(0x10000);
     bytes3 b6 = bytes3(0xFFFF);
     bytes3 b7 = bytes3(0x10000);
     bytes3 b8 = bytes3(0x1000000);
     bytes4 b9 = bytes4(0xFFFFFF);
     bytes4 b10 = bytes4(0x1000000);
     bytes4 b11 = bytes4(0x100000000);
     bytes16 b12 = bytes16(0x1);
     bytes32 b13 = bytes32(0x1);
   }
}
// ----
// TypeError: (60-71): Explicit type conversion not allowed from "int_const 1" to "bytes1".
// TypeError: (90-103): Explicit type conversion not allowed from "int_const 256" to "bytes1".
// TypeError: (122-134): Explicit type conversion not allowed from "int_const 255" to "bytes2".
// TypeError: (153-166): Explicit type conversion not allowed from "int_const 256" to "bytes2".
// TypeError: (185-200): Explicit type conversion not allowed from "int_const 65536" to "bytes2".
// TypeError: (219-233): Explicit type conversion not allowed from "int_const 65535" to "bytes3".
// TypeError: (252-267): Explicit type conversion not allowed from "int_const 65536" to "bytes3".
// TypeError: (286-303): Explicit type conversion not allowed from "int_const 16777216" to "bytes3".
// TypeError: (322-338): Explicit type conversion not allowed from "int_const 16777215" to "bytes4".
// TypeError: (358-375): Explicit type conversion not allowed from "int_const 16777216" to "bytes4".
// TypeError: (395-414): Explicit type conversion not allowed from "int_const 4294967296" to "bytes4".
// TypeError: (435-447): Explicit type conversion not allowed from "int_const 1" to "bytes16".
// TypeError: (468-480): Explicit type conversion not allowed from "int_const 1" to "bytes32".
