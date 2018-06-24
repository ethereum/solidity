contract base { uint baseMember; struct BaseType { uint element; } }
contract derived is base {
    BaseType data;
    function f() public { baseMember = 7; }
}
