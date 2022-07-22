contract C {
    function f() pure public {
        hex"abcd" keccak256;
        1 blockhash;
        true assert;
        0x1234567890123456789012345678901234567890 selfdestruct;
        1 gasleft;
    }
}
// ----
// TypeError 4438: (52-71): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (81-92): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (102-113): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (123-178): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
// TypeError 4438: (188-197): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
