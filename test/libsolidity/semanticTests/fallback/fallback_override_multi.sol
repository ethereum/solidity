contract A {
    fallback (bytes calldata _input) virtual external returns (bytes memory) {
        return _input;
    }
}
contract B {
    fallback (bytes calldata _input) virtual external returns (bytes memory) {
        return "xyz";
    }
}
contract C is B, A {
    fallback () external override (B, A) {}
    function f() public returns (bool, bytes memory) {
        (bool success, bytes memory retval) = address(this).call("abc");
        return (success, retval);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f() -> 0x01, 0x40, 0x00
