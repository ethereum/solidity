contract C {
    mapping (string => uint) m_nameToRecord;
    function set(string calldata key, uint value) external {
        m_nameToRecord[key] = value;
    }
    function get(string calldata key) external view returns (uint) {
        return m_nameToRecord[key];
    }
    function setFixed(uint value) external {
        m_nameToRecord["fixed"] = value;
    }
    function getFixed() external view returns (uint) {
        return m_nameToRecord["fixed"];
    }
}
// ----
// set(string,uint256): 0x40, 8, 3, "abc" ->
// get(string): 0x20, 3, "abc" -> 8
// get(string): 0x20, 3, "abe" -> 0
// getFixed() -> 0
// setFixed(uint256): 9 ->
// getFixed() -> 9
