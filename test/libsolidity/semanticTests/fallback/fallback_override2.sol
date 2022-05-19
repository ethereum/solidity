contract A {
    fallback (bytes calldata _input) virtual external returns (bytes memory) {
        return _input;
    }
}
contract B is A {
    fallback () override external {
    }
    function f() public returns (bool, bytes memory) {
        (bool success, bytes memory retval) = address(this).call("abc");
        return (success, retval);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f() -> 1, 0x40, 0x00
