contract C {
    uint[] x;
    function() external { 
        uint[] storage y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
    }
}
// ----
