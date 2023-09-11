contract C {
    function f() public pure returns (uint) {
        /// @title example of title
        /// @author example of author
        /// @notice example of notice
        /// @dev example of dev
        /// @param example of param
        /// @return example of return
        uint state = 42;
        return state;
    }
}
// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :C userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
