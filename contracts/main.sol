// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

// Lock
contract Lock {
    bool transient private unlocked;

    function unlock() internal {
        unlocked = true;
    }

    function lock() internal {
        unlocked = false;
    }

    function isUnlocked() internal view returns (bool) {
        return unlocked;
    }
}

// PoolManager
contract PoolManager {
    type PoolId is bytes32;
    type Currency is address;

    struct Slot0 {
        uint160 sqrtPriceX96;
        int24 tick;
        uint16 protocolFee;
        uint24 swapFee;
    }

    struct TickInfo {
        uint128 liquidityGross;
        int128 liquidityNet;
        uint256 feeGrowthOutside0X128;
        uint256 feeGrowthOutside1X128;
    }

    struct Info {
        uint128 liquidity;
        uint256 feeGrowthInside0LastX128;
        uint256 feeGrowthInside1LastX128;
    }

    struct State {
        Slot0 slot0;
        uint256 feeGrowthGlobal0X128;
        uint256 feeGrowthGlobal1X128;
        uint128 liquidity;
        mapping(int24 => TickInfo) ticks;
        mapping(int16 => uint256) tickBitmap;
        mapping(bytes32 => Info) positions;
    }

    mapping(address caller => mapping(Currency currency => int256 currencyDelta)) public transient currencyDelta;
    mapping(PoolId id => State) public pools;
}