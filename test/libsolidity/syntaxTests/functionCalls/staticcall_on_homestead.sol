contract C {
    function f() public {
        (bool success, ) = address(10).staticcall{gas: 3}("");
        success;
    }
}
// ====
// EVMVersion: <byzantium
// ----
// TypeError 5052: (66-100): "staticcall" is not supported by the VM version.
