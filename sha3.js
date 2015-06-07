var chai = require('chai');
var assert = chai.assert;
var sha3 = require('../lib/utils/sha3');
var web3 = require('../index');

describe('lib/utils/sha3', function () {
    var test = function (v, e) {
        it('should encode ' + v + ' to ' + e, function () {
            assert.equal(sha3(v), e);
        });
    };

    test('test123', 'f81b517a242b218999ec8eec0ea6e2ddbef2a367a14e93f4a32a39e260f686ad');
    test('test(int)', 'f4d03772bec1e62fbe8c5691e1a9101e520e8f8b5ca612123694632bf3cb51b1');
    test(web3.fromAscii('test123'), 'f81b517a242b218999ec8eec0ea6e2ddbef2a367a14e93f4a32a39e260f686ad');
});

