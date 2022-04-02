contract C {
    function f() public returns (address) {
        return address(f);
    }
}
// ----
// TypeError 9640: (72-82='address(f)'): Explicit type conversion not allowed from "function () returns (address)" to "address".
