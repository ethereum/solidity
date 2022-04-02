contract C {
    function f() public {
        [1, 2, 3]++;
        [1, 2, 3] = [4, 5, 6];
    }
}
// ----
// TypeError 3025: (47-56='[1, 2, 3]'): Inline array type cannot be declared as LValue.
