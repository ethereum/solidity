var assert = require('assert');
var contract = require('../lib/web3/contract.js');
var FakeHttpProvider = require('./helpers/FakeHttpProvider');
var web3 = require('../index');

describe('web3.eth.contract', function() {
    it('should create simple contract with one method from abi with explicit type name', function () {
        
        // given
        var description =  [{
            "name": "test(uint256)",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';
    
        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']);
    });

    it('should create simple contract with one method from abi with implicit type name', function () {
    
        // given
        var description =  [{
            "name": "test",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';

        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']);
    }); 

    it('should create contract with multiple methods', function () {
        
        // given
        var description = [{
            "name": "test",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ],
        }, {
            "name": "test2",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';
        
        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']);
        assert.equal('function', typeof myCon.test2); 
        assert.equal('function', typeof myCon.test2['uint256']);
    });

    it('should create contract with overloaded methods', function () {
    
        // given
        var description = [{
            "name": "test",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ],
        }, {
            "name": "test",
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "string"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';
        
        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']);
        assert.equal('function', typeof myCon.test['string']); 
    });

    it('should create contract with no methods', function () {
        
        // given
        var description =  [{
            "name": "test(uint256)",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';

        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('undefined', typeof myCon.test); 

    });

    it('should create contract with one event', function () {
        
        // given
        var description =  [{
            "name": "test",
            "type": "event",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ],
            "outputs": [
            {
                "name": "d",
                "type": "uint256"
            }
            ]
        }];
        var address = '0x1234567890123456789012345678901234567890';

        // when
        var myCon = contract(description).at(address);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']); 

    });

    it('should create contract with nondefault constructor', function (done) {
        var provider = new FakeHttpProvider();
        web3.setProvider(provider);
        web3.reset(); // reset different polls
        var address = '0x1234567890123456789012345678901234567890';
        var code = '0x31241231231123123123123121cf121212i123123123123123512312412512111111';
        var description =  [{
            "name": "test",
            "type": "constructor",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }
            ]
        }];

        provider.injectResult(address);
        provider.injectValidation(function (payload) {
            assert.equal(payload.jsonrpc, '2.0');
            assert.equal(payload.method, 'eth_sendTransaction');
            assert.equal(payload.params[0].data, code + '0000000000000000000000000000000000000000000000000000000000000002');
            done();
        });
        
        var myCon = contract(description).new(2, {data: code});
    });
});

