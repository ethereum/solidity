contract C {
        function f() public pure {
                C(bytes20(uint160(0x1234)));
        }
}
// ----
// TypeError 9640: (64-91): Explicit type conversion not allowed from "bytes20" to "contract C".
