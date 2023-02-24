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
// TypeError 4438: (62-71): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (83-92): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (107-113): The literal suffix must be either a subdenomination or a file-level suffix function.
// Warning 5159: (166-178): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
// TypeError 4438: (166-178): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 4438: (190-197): The literal suffix must be either a subdenomination or a file-level suffix function.
