contract C {
	constructor() payable {
		assert(tx.origin >= address(0));
		assert(tx.origin <= address(2**160 - 1));
		assert(tx.gasprice >= 0);
		assert(tx.gasprice <= 2**256 - 1);
		assert(msg.sender >= address(0));
		assert(msg.sender <= address(2**160 - 1));
		assert(msg.value >= 0);
		assert(msg.value <= 2**256 - 1);

		assert(block.coinbase >= address(0));
		assert(block.coinbase <= address(2**160 - 1));
		assert(block.timestamp >= 0);
		assert(block.timestamp <= 2**256 - 1);
		assert(block.chainid >= 0);
		assert(block.chainid <= 2**256 - 1);
		assert(block.difficulty >= 0);
		assert(block.difficulty <= 2**256 - 1);
		assert(block.gaslimit >= 0);
		assert(block.gaslimit <= 2**256 - 1);
		assert(block.number >= 0);
		assert(block.number <= 2**256 - 1);
	}
}

contract D {
	constructor() payable {
		unchecked {
			assert(tx.origin >= address(0));
			assert(tx.origin <= address(2**160 - 1));
			assert(tx.gasprice >= 0);
			assert(tx.gasprice <= 2**256 - 1);
			assert(msg.sender >= address(0));
			assert(msg.sender <= address(2**160 - 1));
			assert(msg.value >= 0);
			assert(msg.value <= 2**256 - 1);

			assert(block.coinbase >= address(0));
			assert(block.coinbase <= address(2**160 - 1));
			assert(block.timestamp >= 0);
			assert(block.timestamp <= 2**256 - 1);
			assert(block.chainid >= 0);
			assert(block.chainid <= 2**256 - 1);
			assert(block.difficulty >= 0);
			assert(block.difficulty <= 2**256 - 1);
			assert(block.gaslimit >= 0);
			assert(block.gaslimit <= 2**256 - 1);
			assert(block.number >= 0);
			assert(block.number <= 2**256 - 1);
		}
	}
}

// ====
// SMTEngine: all
// ----
