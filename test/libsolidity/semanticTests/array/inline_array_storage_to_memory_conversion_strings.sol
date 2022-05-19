contract C {
    string s = "doh";

    function f() public returns (string memory, string memory) {
        string memory t = "ray";
        string[3] memory x = [s, t, "mi"];
        return (x[1], x[2]);
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 0x40, 0x80, 0x3, "ray", 0x2, "mi"
