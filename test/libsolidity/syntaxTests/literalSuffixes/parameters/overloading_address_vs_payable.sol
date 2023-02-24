function suffix(address) pure suffix returns (address) {}
function suffix(address payable) pure suffix returns (address payable) {}

contract C {
    address payable a = 0x1234567890123456789012345678901234567890 suffix;
}
// ----
// TypeError 2998: (74-89): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 7407: (170-219): Type address is not implicitly convertible to expected type address payable.
