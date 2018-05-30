contract C {
    function f() pure public {
        address x = 0xA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
        x;
    }
}
// ----
// SyntaxError: (64-105): This looks like an address but is not exactly 40 hex digits. It is 39 hex digits.
