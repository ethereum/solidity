contract C {
    function f() public {
        [1, 2, 3]++;
        [1, 2, 3] = [4, 5, 6];
    }
}
// ----
// TypeError: (47-56): Inline array type cannot be declared as LValue.
