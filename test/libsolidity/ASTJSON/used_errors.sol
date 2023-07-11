error X();
function f() pure { revert X(); }
contract C {
    error T();
    function h() public { f(); }
}
// ----
