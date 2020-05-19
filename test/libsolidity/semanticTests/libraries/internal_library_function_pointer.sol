library L {
    function f() internal returns (uint) {
        return 66;
    }
}

contract C {
    function g() public returns (uint) {
        function() internal returns(uint) ptr;
        ptr = L.f;
        return ptr();
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 66
