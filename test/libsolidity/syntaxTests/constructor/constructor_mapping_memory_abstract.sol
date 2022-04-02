abstract contract A {
    constructor(mapping(uint => uint) memory a) {}
}
// ----
// TypeError 4061: (38-68='mapping(uint => uint) memory a'): Type mapping(uint256 => uint256) is only valid in storage because it contains a (nested) mapping.
