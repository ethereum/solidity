// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

/**
 * @dev Provides a set of functions to operate with Base64 strings.
 */
library NoAsmBase64 {
    bytes private constant TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        function encode(bytes memory data) internal pure returns (string memory) {
        if (data.length == 0) return "";

        bytes memory table     = TABLE;
        bytes memory result    = new bytes(4 * ((data.length + 2) / 3));
        uint256      resultPtr = 0;

        for (uint256 dataPtr = 0; dataPtr < data.length; dataPtr += 3) {
            uint24 chunk = (                            (uint24(uint8(data[dataPtr + 0])) << 16))
                         + (dataPtr + 1 < data.length ? (uint24(uint8(data[dataPtr + 1])) <<  8) : 0)
                         + (dataPtr + 2 < data.length ? (uint24(uint8(data[dataPtr + 2]))      ) : 0);

            result[resultPtr++] = table[uint8(chunk >> 18) & 0x3f];
            result[resultPtr++] = table[uint8(chunk >> 12) & 0x3f];
            result[resultPtr++] = table[uint8(chunk >>  6) & 0x3f];
            result[resultPtr++] = table[uint8(chunk      ) & 0x3f];
        }

        if (data.length % 3 == 1) {
            result[--resultPtr] = 0x3d;
            result[--resultPtr] = 0x3d;
        }
        else if (data.length % 3 == 2) {
            result[--resultPtr] = 0x3d;
        }

        return (string(result));
    }
}
