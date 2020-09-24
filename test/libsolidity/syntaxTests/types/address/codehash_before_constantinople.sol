contract C {
    function f() public view returns (bytes32) {
        return address(this).codehash;
    }
}
// ====
// EVMVersion: <constantinople
// ----
// TypeError 7598: (77-99): "codehash" is not supported by the VM version.
