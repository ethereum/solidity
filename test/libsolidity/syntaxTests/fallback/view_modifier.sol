contract C {
    uint x;
    function() external view { x = 2; }
}
// ----
// TypeError: (29-64): Fallback function must be payable or non-payable, but is "view".
