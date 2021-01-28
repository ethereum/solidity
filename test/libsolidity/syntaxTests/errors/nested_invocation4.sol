error E1(uint);
error E2();
function f() pure {
    revert(E1([E2()]));
}
// ----
// TypeError 5604: (63-67): Array component cannot be empty.
