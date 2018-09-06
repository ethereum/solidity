contract C {
    function f() public view returns (address) {
        return address(this.f);
    }
}
