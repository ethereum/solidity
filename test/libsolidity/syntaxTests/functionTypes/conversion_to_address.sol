contract C {
    function f() public view returns (address) {
        return address(this.f);
    }
}
// ----
// TypeError: (77-92): Explicit type conversion not allowed from "function () view external returns (address)" to "address".
