contract C {
        function f() public pure {
                C(bytes20(0x1234));
        }
}
// ----
// TypeError: (64-82): Explicit type conversion not allowed from "bytes20" to "contract C".
