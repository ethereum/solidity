//sol FixedFeeRegistrar
// Simple global registrar with fixed-fee reservations.
// @authors:
//   Gav Wood <g@ethdev.com>

pragma solidity >=0.4.0 <0.9.0;

abstract contract Registrar {
	event Changed(string indexed name);

	function owner(string memory _name) public virtual view returns (address o_owner);
	function addr(string memory _name) public virtual view returns (address o_address);
	function subRegistrar(string memory _name) virtual public view returns (address o_subRegistrar);
	function content(string memory _name) public virtual view returns (bytes32 o_content);
}

contract FixedFeeRegistrar is Registrar {
	struct Record {
		address addr;
		address subRegistrar;
		bytes32 content;
		address owner;
	}

	modifier onlyrecordowner(string memory _name) { if (m_record(_name).owner == msg.sender) _; }

	function reserve(string memory _name) public payable {
		Record storage rec = m_record(_name);
		if (rec.owner == 0x0000000000000000000000000000000000000000 && msg.value >= c_fee) {
			rec.owner = msg.sender;
			emit Changed(_name);
		}
	}
	function disown(string memory _name, address payable _refund) onlyrecordowner(_name) public {
		delete m_recordData[uint(keccak256(bytes(_name))) / 8];
		if (!_refund.send(c_fee))
			revert();
		emit Changed(_name);
	}
	function transfer(string memory _name, address _newOwner) onlyrecordowner(_name) public {
		m_record(_name).owner = _newOwner;
		emit Changed(_name);
	}
	function setAddr(string memory _name, address _a) onlyrecordowner(_name) public {
		m_record(_name).addr = _a;
		emit Changed(_name);
	}
	function setSubRegistrar(string memory _name, address _registrar) onlyrecordowner(_name) public {
		m_record(_name).subRegistrar = _registrar;
		emit Changed(_name);
	}
	function setContent(string memory _name, bytes32 _content) onlyrecordowner(_name) public {
		m_record(_name).content = _content;
		emit Changed(_name);
	}

	function record(string memory _name) public view returns (address o_addr, address o_subRegistrar, bytes32 o_content, address o_owner) {
		Record storage rec = m_record(_name);
		o_addr = rec.addr;
		o_subRegistrar = rec.subRegistrar;
		o_content = rec.content;
		o_owner = rec.owner;
	}
	function addr(string memory _name) public override view returns (address) { return m_record(_name).addr; }
	function subRegistrar(string memory _name) public override view returns (address) { return m_record(_name).subRegistrar; }
	function content(string memory _name) public override view returns (bytes32) { return m_record(_name).content; }
	function owner(string memory _name) public override view returns (address) { return m_record(_name).owner; }

	Record[2**253] m_recordData;
	function m_record(string memory _name) view internal returns (Record storage o_record) {
		return m_recordData[uint(keccak256(bytes(_name))) / 8];
	}
	uint constant c_fee = 69 ether;
}
// ====
// compileViaYul: also
// ----
// constructor()
// gas irOptimized: 425623
// gas legacy: 936897
// gas legacyOptimized: 490983
// reserve(string), 69 ether: 0x20, 3, "abc" ->
// ~ emit Changed(string): #0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45
// gas irOptimized: 45967
// gas legacy: 46842
// gas legacyOptimized: 46091
// owner(string): 0x20, 3, "abc" -> 0x1212121212121212121212121212120000000012
// reserve(string), 70 ether: 0x20, 3, "def" ->
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// owner(string): 0x20, 3, "def" -> 0x1212121212121212121212121212120000000012
// reserve(string), 68 ether: 0x20, 3, "ghi" ->
// owner(string): 0x20, 3, "ghi" -> 0
// account: 1 -> 0x1212121212121212121212121212120000001012
// reserve(string), 69 ether: 0x20, 3, "abc" ->
// owner(string): 0x20, 3, "abc" -> 0x1212121212121212121212121212120000000012
// account: 0 -> 0x1212121212121212121212121212120000000012
// setContent(string,bytes32): 0x40, 0, 3, "abc" ->
// ~ emit Changed(string): #0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45
// transfer(string,address): 0x40, 555, 3, "abc" ->
// ~ emit Changed(string): #0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45
// owner(string): 0x20, 3, "abc" -> 555
// content(string): 0x20, 3, "abc" -> 0x00
// setContent(string,bytes32): 0x40, 333, 3, "def" ->
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// setAddr(string,address): 0x40, 124, 3, "def" ->
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// setSubRegistrar(string,address): 0x40, 125, 3, "def" ->
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// content(string): 0x20, 3, "def" -> 333
// addr(string): 0x20, 3, "def" -> 124
// subRegistrar(string): 0x20, 3, "def" -> 125
// balance: 0x124 -> 0
// disown(string,address): 0x40, 0x124, 3, "def" ->
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// balance: 0x124 -> 0
// ~ emit Changed(string): #0x34607c9bbfeb9c23509680f04363f298fdb0b5f9abe327304ecd1daca08cda9c
// owner(string): 0x20, 3, "def" -> 0
// content(string): 0x20, 3, "def" -> 0
// addr(string): 0x20, 3, "def" -> 0
// subRegistrar(string): 0x20, 3, "def" -> 0
