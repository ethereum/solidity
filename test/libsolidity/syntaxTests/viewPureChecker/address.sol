contract C {
    function f() public view returns (uint) {
        return address(this).balance;
    }
    function g() public view returns (uint) {
        return address(0).balance;
    }
    function h() public view returns (bytes32) {
        return address(0).codehash;
    }
}
