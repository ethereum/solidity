// SPDX-License-Identifier: GPL-3.0


// Regression for handling signedness, see issues #14791 and #14792
contract C {
    mapping(bool => int240) internal v1;
    mapping(bytes14 => bytes15) internal v;

    function f() public payable {
        delete v["A"];
    }
}
