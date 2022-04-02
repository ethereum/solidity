library L { function l() public {} }
contract test {
    function f() public {
        L x;
        x.l();
    }
}
// ----
// TypeError 1130: (87-88='L'): Invalid use of a library name.
