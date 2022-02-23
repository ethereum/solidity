contract C {
    function f() public view returns (bytes32) {
        return address(this).codehash;
    }
}
// ====
// EVMVersion: >=constantinople
// ----
