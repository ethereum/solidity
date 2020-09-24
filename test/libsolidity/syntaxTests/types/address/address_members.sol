contract C {
    function f() public view returns (address) { return address(this); }
    function g() public view returns (uint) { return f().balance; }
    function h() public view returns (bytes memory) { return f().code; }
    function i() public view returns (uint) { return f().code.length; }
    function j() public view returns (uint) { return h().length; }
}
// ----
