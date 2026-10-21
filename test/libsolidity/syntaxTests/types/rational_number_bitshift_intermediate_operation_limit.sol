contract C {
    function f() public pure {
        int a;
        a = (1 << 5000) >> 4990;
        a = (1 << 4097) >> 4090;
    }
}
// ----
// TypeError 2271: (72-81): Built-in binary operator << cannot be applied to types int_const 1 and int_const 5000.
// TypeError 2271: (105-114): Built-in binary operator << cannot be applied to types int_const 1 and int_const 4097.
