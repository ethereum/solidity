contract C {
    address view m_a;
    address pure m_b;
    address view[] m_c;
    mapping(uint => address view) m_d;
    function f() public pure {
        address view a;
        address pure b;
        a; b;
    }
    function g(address view) public pure {}
    function h(address pure) public pure {}
    function i() public pure returns (address view) {}
    function j() public pure returns (address pure) {}
    modifier m1(address view) {_;}
    modifier m2(address pure) {_;}
    event e1(address view);
    event e2(address pure);
    error err1(address view);
    error err2(address pure);
    function f2() public pure returns (address) {
        try this.f2() returns (address view res) {} catch {}
    }
    function f3() public pure returns (address) {
        try this.f3() returns (address pure res) {} catch {}
    }
}
address view constant f_a;
address pure constant f_b;
// ----
// TypeError 2311: (17-29): Address types can only be payable or non-payable.
// TypeError 2311: (39-51): Address types can only be payable or non-payable.
// TypeError 2311: (61-73): Address types can only be payable or non-payable.
// TypeError 2311: (101-113): Address types can only be payable or non-payable.
// TypeError 2311: (159-171): Address types can only be payable or non-payable.
// TypeError 2311: (183-195): Address types can only be payable or non-payable.
// TypeError 2311: (234-246): Address types can only be payable or non-payable.
// TypeError 2311: (278-290): Address types can only be payable or non-payable.
// TypeError 2311: (345-357): Address types can only be payable or non-payable.
// TypeError 2311: (400-412): Address types can only be payable or non-payable.
// TypeError 2311: (433-445): Address types can only be payable or non-payable.
// TypeError 2311: (468-480): Address types can only be payable or non-payable.
// TypeError 2311: (500-512): Address types can only be payable or non-payable.
// TypeError 2311: (528-540): Address types can only be payable or non-payable.
// TypeError 2311: (558-570): Address types can only be payable or non-payable.
// TypeError 2311: (588-600): Address types can only be payable or non-payable.
// TypeError 2311: (684-696): Address types can only be payable or non-payable.
// TypeError 2311: (801-813): Address types can only be payable or non-payable.
// TypeError 2311: (839-851): Address types can only be payable or non-payable.
// TypeError 2311: (866-878): Address types can only be payable or non-payable.
