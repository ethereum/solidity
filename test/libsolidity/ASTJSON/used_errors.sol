error X();
function f() { revert X(); }
contract C {
    error T();
    function h() public pure { f(); }
}

// ----
