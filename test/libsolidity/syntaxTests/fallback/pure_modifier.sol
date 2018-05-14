contract C {
    uint x;
    function() pure { x = 2; }
}
// ----
// TypeError: (29-55): Fallback function must be payable or non-payable, but is "pure".
