contract c {
    mapping(uint => uint) y;
    function f() view public {
        mapping(uint => uint) memory x = y;
        x;
    }
}
// ----
// TypeError 4061: (81-111): Type mapping(uint256 => uint256) is only valid in storage because it contains a (nested) mapping.
