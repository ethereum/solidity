var chai = require('chai');
var assert = chai.assert;
var web3 = require('../index');

describe('web3.eth', function () {
    describe('defaultBlock', function () {
        it('should check if defaultBlock is set to proper value', function () {
            assert.equal(web3.eth.defaultBlock, 'latest');
        });
    });
});

