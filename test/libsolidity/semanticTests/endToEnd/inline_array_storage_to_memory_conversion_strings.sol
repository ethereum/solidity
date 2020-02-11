contract C {
    string s = "doh";

    function f() public returns(string memory, string memory) {
        string memory t = "ray";
        string[3] memory x = [s, t, "mi"];
        return (x[1], x[2]);
    }
}

// ----
// f() -> 0x40, 0x80, 3, string("ray", 2, string("mi"
// f():"" -> "64, 128, 3, ray, 2, mi"
