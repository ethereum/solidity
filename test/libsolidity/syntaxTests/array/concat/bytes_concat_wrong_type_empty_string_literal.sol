contract C {
    function f() public pure {
        bytes.concat(hex"", unicode"", "");
    }
}
// ----
