var chai = require('chai');
var utils = require('../lib/utils/utils');
var assert = chai.assert;

describe('lib/utils/utils', function () {
    describe('toWei', function () {
        it('should return the correct value', function () {
            
            assert.equal(utils.toWei(1, 'wei'),    '1');
            assert.equal(utils.toWei(1, 'kwei'),   '1000');
            assert.equal(utils.toWei(1, 'mwei'),   '1000000');
            assert.equal(utils.toWei(1, 'gwei'),   '1000000000');
            assert.equal(utils.toWei(1, 'szabo'),  '1000000000000');
            assert.equal(utils.toWei(1, 'finney'), '1000000000000000');
            assert.equal(utils.toWei(1, 'ether'),  '1000000000000000000');
            assert.equal(utils.toWei(1, 'kether'), '1000000000000000000000');
            assert.equal(utils.toWei(1, 'grand'),  '1000000000000000000000');
            assert.equal(utils.toWei(1, 'mether'), '1000000000000000000000000');
            assert.equal(utils.toWei(1, 'gether'), '1000000000000000000000000000');
            assert.equal(utils.toWei(1, 'tether'), '1000000000000000000000000000000');

            assert.equal(utils.toWei(1, 'kwei'),    utils.toWei(1, 'femtoether'));
            assert.equal(utils.toWei(1, 'babbage'), utils.toWei(1, 'picoether'));
            assert.equal(utils.toWei(1, 'shannon'), utils.toWei(1, 'nanoether'));
            assert.equal(utils.toWei(1, 'szabo'),   utils.toWei(1, 'microether'));
            assert.equal(utils.toWei(1, 'finney'),  utils.toWei(1, 'milliether'));
            assert.equal(utils.toWei(1, 'milli'),    utils.toWei(1, 'milliether'));
            assert.equal(utils.toWei(1, 'milli'),    utils.toWei(1000, 'micro'));

            assert.throws(function () {utils.toWei(1, 'wei1');}, Error);
        });
    });
});
