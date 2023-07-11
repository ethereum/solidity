contract C {
    mapping(uint => uint) private m0;
    mapping(uint => uint) private m1;
    mapping(uint => uint) private m2;

    function f(uint i) public returns (uint slot, uint offset) {
        mapping(uint => uint) storage m0Ptr = m0;
        mapping(uint => uint) storage m1Ptr = m1;
        mapping(uint => uint) storage m2Ptr = m2;

        assembly {
            switch i
            case 1 {
                slot := m1Ptr.slot
                offset := m1Ptr.offset
            }
            case 2 {
                slot := m2Ptr.slot
                offset := m2Ptr.offset
            }
            default {
                slot := m0Ptr.slot
                offset := m0Ptr.offset
            }
        }
    }
}
// ----
// f(uint256): 0 -> 0, 0
// f(uint256): 1 -> 1, 0
// f(uint256): 2 -> 2, 0
