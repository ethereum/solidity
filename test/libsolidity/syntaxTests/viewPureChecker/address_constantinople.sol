contract C {
    function f() public view returns (bytes32) {
        return address(this).codehash;
    }
    function g() public view returns (bytes32) {
        return address(0).codehash;
    }
}
// ====
// EVMVersion: >=constantinople
// ----
