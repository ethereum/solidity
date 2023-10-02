contract C {
    uint immutable public x = 42;

    function g() external view returns (uint) {}

    function f() public view returns (uint) {
        return this.x();
    }

    function h() public view returns (function () external view returns (uint)) {
        return this.x;
    }
}
