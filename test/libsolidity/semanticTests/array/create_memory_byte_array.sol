contract C {
    function f() public returns (bytes1) {
        bytes memory x = new bytes(35);
        assert(x.length == 35);
        x[34] = "A";
        return (x[34]);
    }
}
// ----
// f() -> "A"
