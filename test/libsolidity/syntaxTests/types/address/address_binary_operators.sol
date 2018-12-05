contract C {
    address a;
    function f() public pure returns(bool) {
        a = address(0) + address(0);
        a = address(0) - address(0);
        a = address(0) * address(0);
        a = address(0) / address(0);
        return address(0) == address(0);
    }
}
// ----
// TypeError: (85-108): Operator + not compatible with types address payable and address payable. Addresses can only be compared
// TypeError: (122-145): Operator - not compatible with types address payable and address payable. Addresses can only be compared
// TypeError: (159-182): Operator * not compatible with types address payable and address payable. Addresses can only be compared
// TypeError: (196-219): Operator / not compatible with types address payable and address payable. Addresses can only be compared
