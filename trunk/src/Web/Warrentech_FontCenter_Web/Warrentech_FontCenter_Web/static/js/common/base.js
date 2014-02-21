String.format = function () {
	if (arguments.length == 0)
		return null;

	var str = arguments[0];
	for (var i = 1; i < arguments.length; i++) {
		var arg = arguments[i];
		if (arg == undefined || arg == null) { arg = '' }
		str = str.split('{' + (i - 1) + '}').join(arg);
	}
	return str;
};

Array.prototype.indexOf = function (substr, start) {
	var ta, rt, d = '\0';
	if (start != null) { ta = this.slice(start); rt = start; } else { ta = this; rt = 0; }
	var str = d + ta.join(d) + d, t = str.indexOf(d + substr + d);
	if (t == -1) return -1; rt += str.slice(0, t).replace(/[^\0]/g, '').length;
	return rt;
};

String.prototype.ASCIIHash = function () {
	var text_to_insert = [];
	var str = this.split('');
	for (i = 0; i < str.length; i++) {
		text_to_insert.push(str[i].charCodeAt());
	}
	return text_to_insert.join('');
};

String.prototype.toJson = function () {
	return eval('(' + this + ')');
};

Date.formatDate = function (date) {
	return new Date(date.replace(/\s(\+|\-)/, ' UTC$1'));
};

String.prototype.ASCIILength = function () {
	var count = 0;
	for (var i = 0; i < this.length; i++) {
		if (this.charCodeAt(i) > 255) {
			count += 2;
		} else {
			count++;
		}
	}
	return count;
};

String.prototype.hashCode = function () {
	var hash = 0;
	if (this.length == 0) return hash;
	for (i = 0; i < this.length; i++) {
		char = this.charCodeAt(i);
		hash = ((hash << 5) - hash) + char;
		hash = hash & hash; // Convert to 32bit integer
	}
	return hash;
};

Array.prototype.uniq = function () {

	var temp = {}, len = this.length;

	for (var i = 0; i < len; i++) {

		if (typeof temp[this[i]] == "undefined") {

			temp[this[i]] = 1;

		}

	}

	this.length = 0;

	len = 0;

	for (var i in temp) {

		this[len++] = i;

	}

	return this;

};

String.prototype.trim = function () {
	return this.replace(/^\s\s*/, '').replace(/\s\s*$/, '');
};

String.prototype.escapeChar = function () {
	return this.replace(/([\.\?\+\*\^\$\[\]\|\(\)\\])/gm, '\\$1');
};

String.prototype.parseDate = function () {
	var formatedDate = this.replace(/\s/g, '').replace(/-/g, '/');
	return new Date(formatedDate);
}

function hashCode(o) {
	if (typeof o != 'object' || o instanceof Array) {
		return o.toString().hashCode();
	}

	var valueArray = [];
	var ignore = false;
	for (var key in o) {
		if (typeof o[key] !== 'string') {
			continue;
		}
		for (var i = 1; i < arguments.length; i++) {
			if (key == arguments[i]) {
				ignore = true;
				break;
			}
		}
		if (ignore) {
			ignore = false;
			continue;
		}
		valueArray.push(o[key]);
	}
	return valueArray.join().hashCode();
}