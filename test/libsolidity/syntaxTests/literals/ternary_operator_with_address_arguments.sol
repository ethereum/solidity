contract C {
    function f(bool cond) public {
         // OK
         address a1 = cond ? 0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF : payable(0x1234567890123456789012345678901234567890);
         address a2 = cond ? address(0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF) : 0x1234567890123456789012345678901234567890;

         // Errors
         address payable a3 = cond ? 0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF : payable(0x1234567890123456789012345678901234567890);
         cond ? 0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF : 1;
    }
}
// ----
// TypeError 9574: (346-470): Type address is not implicitly convertible to expected type address payable.
// TypeError 1080: (481-534): True expression's type address does not match false expression's type uint8.
