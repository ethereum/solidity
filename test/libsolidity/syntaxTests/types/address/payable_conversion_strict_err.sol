contract C {
    function f() public pure {
        address payable a = payable(uint160(0));
        address payable b = payable(bytes20(0));
        address payable c = payable(2);
        // hex literal that is only 15 bytes long
        address payable d = payable(0x002190356cBB839Cbe05303d7705Fa);

        // The opposite should also be disallowed
        uint160 a1 = uint160(payable(0));
        bytes20 b1 = bytes20(payable(0));
    }
}
// ----
// TypeError 9640: (72-91): Explicit type conversion not allowed from "uint160" to "address payable".
// TypeError 9640: (121-140): Explicit type conversion not allowed from "bytes20" to "address payable".
// TypeError 9640: (170-180): Explicit type conversion not allowed from "int_const 2" to "address payable".
// TypeError 9640: (260-301): Explicit type conversion not allowed from "int_const 6807...(25 digits omitted)...4970" to "address payable".
// TypeError 9640: (375-394): Explicit type conversion not allowed from "address payable" to "uint160".
// TypeError 9640: (417-436): Explicit type conversion not allowed from "address payable" to "bytes20".
