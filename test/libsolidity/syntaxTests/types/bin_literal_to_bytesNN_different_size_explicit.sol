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
// TypeError 9640: (90-109): Explicit type conversion not allowed from "int_const 256" to "bytes1".
// TypeError 9640: (128-146): Explicit type conversion not allowed from "int_const 255" to "bytes2".
// TypeError 9640: (165-184): Explicit type conversion not allowed from "int_const 256" to "bytes2".
// TypeError 9640: (203-230): Explicit type conversion not allowed from "int_const 65536" to "bytes2".
// TypeError 9640: (249-275): Explicit type conversion not allowed from "int_const 65535" to "bytes3".
// TypeError 9640: (294-321): Explicit type conversion not allowed from "int_const 65536" to "bytes3".
// TypeError 9640: (340-375): Explicit type conversion not allowed from "int_const 16777216" to "bytes3".
// TypeError 9640: (394-428): Explicit type conversion not allowed from "int_const 16777215" to "bytes4".
// TypeError 9640: (448-483): Explicit type conversion not allowed from "int_const 16777216" to "bytes4".
// TypeError 9640: (503-546): Explicit type conversion not allowed from "int_const 4294967296" to "bytes4".
// TypeError 9640: (567-579): Explicit type conversion not allowed from "int_const 1" to "bytes16".
// TypeError 9640: (600-612): Explicit type conversion not allowed from "int_const 1" to "bytes32".
