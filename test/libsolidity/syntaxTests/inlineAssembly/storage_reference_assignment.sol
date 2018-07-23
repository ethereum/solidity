contract C {
    uint[] x;
    function() external { 
        uint[] storage y = x;
        assembly {
            y_slot := 1
            y_offset := 2
        }
    }
}
// ----
// TypeError: (115-121): Storage variables cannot be assigned to.
// TypeError: (139-147): Storage variables cannot be assigned to.
