abstract contract I {
    function a() internal view virtual returns(uint256);
}

abstract contract C is I {
    function f() public view returns(uint256) {
        return I.a();
    }
}

abstract contract D is I {
    function f() public view returns(uint256) {
        return super.a();
    }
}
// ----
// TypeError 7501: (172-177='I.a()'): Cannot call unimplemented base function.
// TypeError 9582: (278-285='super.a'): Member "a" not found or not visible after argument-dependent lookup in type(contract super D).
