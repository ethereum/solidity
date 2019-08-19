contract C {
    function f() public view returns (address) {
        return this.f.address;
    }
}
