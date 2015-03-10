var assert = require('assert');
var contract = require('../lib/web3/contract.js');

describe('contract', function() {
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
    
        // when
        var Con = contract(description);
        var myCon = new Con(null);

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

        // when
        var Con = contract(description);
        var myCon = new Con(null);

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
        
        // when
        var Con = contract(description);
        var myCon = new Con(null);

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
        
        // when
        var Con = contract(description);
        var myCon = new Con(null);

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


        // when
        var Con = contract(description);
        var myCon = new Con(null);

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


        // when
        var Con = contract(description);
        var myCon = new Con(null);

        // then
        assert.equal('function', typeof myCon.test); 
        assert.equal('function', typeof myCon.test['uint256']); 

    });

});

