struct S {
    uint x;
}

enum E {A, B, C}

contract C {
    function f() public pure {
        address(uint);
        address(bytes16);
        address(bool);
        address(address);
        // FIXME when fixed point types get implemented
        // address(fixed);

        address(S);
        address(E);

        address(uint[]);
        address(uint[][]);
        address(uint[5]);
        address(string);
        address(bytes);
        address(S[]);
        address(E[]);
        address((uint, uint));

        address(type(uint));
    }
}
// ----
// TypeError 9640: (96-109): Explicit type conversion not allowed from "type(uint256)" to "address".
// TypeError 9640: (119-135): Explicit type conversion not allowed from "type(bytes16)" to "address".
// TypeError 9640: (145-158): Explicit type conversion not allowed from "type(bool)" to "address".
// TypeError 9640: (168-184): Explicit type conversion not allowed from "type(address)" to "address".
// TypeError 9640: (278-288): Explicit type conversion not allowed from "type(struct S storage pointer)" to "address".
// TypeError 9640: (298-308): Explicit type conversion not allowed from "type(enum E)" to "address".
// TypeError 9640: (319-334): Explicit type conversion not allowed from "type(uint256[] memory)" to "address".
// TypeError 9640: (344-361): Explicit type conversion not allowed from "type(uint256[] memory[] memory)" to "address".
// TypeError 9640: (371-387): Explicit type conversion not allowed from "type(uint256[5] memory)" to "address".
// TypeError 9640: (397-412): Explicit type conversion not allowed from "type(string storage pointer)" to "address".
// TypeError 9640: (422-436): Explicit type conversion not allowed from "type(bytes storage pointer)" to "address".
// TypeError 9640: (446-458): Explicit type conversion not allowed from "type(struct S memory[] memory)" to "address".
// TypeError 9640: (468-480): Explicit type conversion not allowed from "type(enum E[] memory)" to "address".
// TypeError 9640: (490-511): Explicit type conversion not allowed from "tuple(type(uint256),type(uint256))" to "address".
// TypeError 9640: (522-541): Explicit type conversion not allowed from "type(uint256)" to "address".
