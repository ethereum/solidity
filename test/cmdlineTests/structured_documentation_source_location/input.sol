// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0;
abstract contract C {
    /// @param id Some identifier
    /// @return No value returned
    function vote(uint id) public virtual returns (uint value);

    /// @return No value returned
    function unvote(uint id) public virtual returns (uint value);
}