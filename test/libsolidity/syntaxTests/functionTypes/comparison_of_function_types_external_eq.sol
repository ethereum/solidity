contract C {
    function f() public view returns (bool ret) {
        return this.f == this.f;
    }
    function g() public view returns (bool ret) {
        return this.f != this.f;
    }
}
// ----
