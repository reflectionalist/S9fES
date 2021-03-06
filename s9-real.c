/*
 * Scheme 9 from Empty Space, big real number arithmetics
 * By Nils M Holm, 2008-2010
 * Placed in the Public Domain
 */

/*
 * Functions and macros with a "_real_" prefix expect data
 * objects of the primitive "real" type. Passing bignums to
 * them will result in mayhem.
 *
 * Functions with a "real_" prefix will delegate integer
 * operations to the corresponding bignum functions and
 * convert mixed arguments to real.
 */

char	*ntoa(char *b, cell n, int w);
cell	bignum_shift_right(cell a);
cell	flat_copy(cell n, cell *lastp);
int	bignum_equal_p(cell a, cell b);
int	bignum_less_p(cell a, cell b);
cell	bignum_multiply(cell a, cell b);
cell	bignum_divide(cell x, cell a, cell b);
cell	bignum_negate(cell a);

#define exponent_char_p(c) \
	(c == 'd' || c == 'D' || \
	 c == 'e' || c == 'E' || \
	 c == 'f' || c == 'F' || \
	 c == 'l' || c == 'L' || \
	 c == 's' || c == 'S')

int string_numeric_p(char *s) {
	int	i;
	int	got_digits = 0,
		got_expn = 0,
		got_point = 0,
		got_sign = 0;

	i = 0;
	if (s[0] == '+' || s[0] == '-') {
		i = 1;
		got_sign = 1;
	}
	if (!s[i])
		return 0;
	while (s[i]) {
		if (exponent_char_p(s[i]) && got_digits && !got_expn) {
			if (isdigit((int) s[i+1]) || s[i+1] == '#') {
				got_expn = 1;
			}
			else if ((s[i+1] == '+' || s[i+1] == '-') &&
				(isdigit((int) s[i+2]) || s[i+2] == '#')
			) {
				got_expn = 1;
				i++;
			}
			else {
				return 0;
			}
		}
		else if (s[i] == '.' && !got_point) {
			got_point = 1;
		}
		else if (s[i] == '#' &&
			 (got_digits || got_point || got_sign)
		) {
			got_digits = 1;
		}
		else if (isdigit((int) s[i])) {
			got_digits = 1;
		}
		else {
			return 0;
		}
		i++;
	}
	return 1;
}

char* replace_hashes(char *s) {
	char	*new;

	new = copy_string(s);
	s = new;
	while (*s) {
		if (*s == '#')
			*s = '5';
		s++;
	}
	return new;
}

cell make_real(int flags, cell exp, cell mant);
cell real_normalize(cell x, char *who);

cell string_to_bignum(char *numstr, int must_be_exact) {
	cell	n, v;
	int	k, j, sign, exact;
	char	*s, *buf;

	buf = replace_hashes(numstr);
	exact = strchr(numstr, '#') == NULL;
	s = buf;
	sign = 1;
	if (s[0] == '-') {
		s++;
		sign = -1;
	}
	else if (s[0] == '+') {
		s++;
	}
	k = (int) strlen(s);
	n = NIL;
	while (k) {
		j = k <= DIGITS_PER_WORD? k: DIGITS_PER_WORD;
		v = atol(&s[k-j]);
		s[k-j] = 0;
		k -= j;
		if (k == 0)
			v *= sign;
		n = new_atom(v, n);
	}
	free(buf);
	if (exact || must_be_exact)
		return new_atom(T_INTEGER, n);
	car(n) = labs(car(n));
	n = make_real((sign<0? REAL_NEGATIVE: 0), 0, n);
	return real_normalize(n, NULL);
}

cell make_integer(cell i);
cell bignum_shift_left(cell a, int fill);
cell bignum_add(cell a, cell b);

cell string_to_real(char *s) {
	cell	mantissa, n;
	cell	exponent;
	int	found_dp;
	int	neg = 0;
	int	i, j, v;

	mantissa = make_integer(0);
	save(mantissa);
	exponent = 0;
	i = 0;
	if (s[i] == '+') {
		i++;
	}
	else if (s[i] == '-') {
		neg = 1;
		i++;
	}
	found_dp = 0;
	while (isdigit((int) s[i]) || s[i] == '#' || s[i] == '.') {
		if (s[i] == '.') {
			i++;
			found_dp = 1;
			continue;
		}
		if (found_dp)
			exponent--;
		mantissa = bignum_shift_left(mantissa, 0);
		car(Stack) = mantissa;
		if (s[i] == '#')
			v = 5;
		else
			v = s[i]-'0';
		mantissa = bignum_add(mantissa, make_integer(v));
		car(Stack) = mantissa;
		i++;
	}
	j = 0;
	for (n = cdr(mantissa); n != NIL; n = cdr(n))
		j++;
	if (exponent_char_p(s[i])) {
		i++;
		n = string_to_bignum(&s[i], 1);
		if (cddr(n) != NIL) {
			unsave(1);
			return error(
				"exponent too big in real number literal",
				make_string(s, strlen(s)));
		}
		exponent += integer_value("", n);
	}
	unsave(1);
	n = make_real((neg? REAL_NEGATIVE: 0),
			exponent, cdr(mantissa));
	return real_normalize(n, NULL);
}

cell string_to_number(char *s) {
	int	i;

	for (i=0; s[i]; i++) {
		if (s[i] == '.' || exponent_char_p(s[i]))
			return string_to_real(s);
	}
	return string_to_bignum(s, 0);
}

cell bignum_read(char *pre, int radix);
cell bignum_abs(cell a);
cell real_to_bignum(cell x);

cell read_real_number(int inexact) {
	cell	n, m;
	int	flags;
	char	buf[50];

	n = read_form(0);
	if (integer_p(n)) {
		if (!inexact)
			return n;
		flags = _bignum_negative_p(n)? REAL_NEGATIVE: 0;
		m = bignum_abs(n);
		n = make_real(flags, 0, cdr(m));
		return real_normalize(n, "numeric literal");
	}
	else if (real_p(n)) {
		if (inexact)
			return n;
		m = real_to_bignum(n);
		if (m == NIL)
			return error("#e: no exact representation for", n);
		return m;
	}
	sprintf(buf, "number expected after #%c, got",
		inexact? 'i': 'e');
	return error(buf, n);
}

void print_expanded_real(cell m, cell e, int n_digits, int neg) {
	char	buf[DIGITS_PER_WORD+3];
	int	k, first;
	int	dp_offset, old_offset;

	dp_offset = e+n_digits;
	if (neg)
		pr("-");
	if (dp_offset <= 0)
		pr("0");
	if (dp_offset < 0)
		pr(".");
	while (dp_offset < 0) {
		pr("0");
		dp_offset++;
	}
	dp_offset = e+n_digits;
	first = 1;
	while (m != NIL) {
		ntoa(buf, labs(car(m)), first? 0: DIGITS_PER_WORD);
		k = strlen(buf);
		old_offset = dp_offset;
		dp_offset -= k;
		if (dp_offset < 0 && old_offset >= 0) {
			memmove(&buf[k+dp_offset+1], &buf[k+dp_offset],
				-dp_offset+1);
			buf[k+dp_offset] = '.';
		}
		pr(buf);
		m = cdr(m);
		first = 0;
	}
	if (dp_offset >= 0) {
		while (dp_offset > 0) {
			pr("0");
			dp_offset--;
		}
		pr(".0");
	}
}

cell count_digits(cell m) {
	int	k = 0;
	cell	x;

	x = car(m);
	k = 0;
	while (x != 0) {
		x /= 10;
		k++;
	}
	k = k==0? 1: k;
	m = cdr(m);
	while (m != NIL) {
		k += DIGITS_PER_WORD;
		m = cdr(m);
	}
	return k;
}

cell round_last(cell m, cell e, int n_digits, int flags) {
	cell	one;
	int	r;

	m = bignum_shift_right(new_atom(T_INTEGER, m));
	r = integer_value("print", cdr(m));
	m = car(m);
	if (r >= 5) {
		save(m);
		one = make_integer(1);
		m = bignum_add(m, one);
		unsave(1);
	}
	m = cdr(m);
	n_digits--;
	e++;
	m = make_real(flags, e, m);
	return real_normalize(m, "print");
}

/* Print real number. */
int print_real(cell n) {
	int	n_digits;
	cell	m, e;
	char	buf[DIGITS_PER_WORD+2];

	if (!real_p(n))
		return 0;
	m = _real_mantissa(n);
	n_digits = count_digits(m);
	e = _real_exponent(n);
	if (n_digits == MANTISSA_SIZE && e < 0) {
		print_real(round_last(m, e, n_digits, _real_flags(n)));
		return 1;
	}
	if (e+n_digits > -4 && e+n_digits <= 6) {
		print_expanded_real(m, e, n_digits, _real_negative_flag(n));
		return 1;
	}
	if (_real_negative_flag(n))
		pr("-");
	ntoa(buf, car(m), 0);
	pr_raw(buf, 1);
	pr(".");
	pr(buf[1] || cdr(m) != NIL? &buf[1]: "0");
	m = cdr(m);
	while (m != NIL) {
		pr(ntoa(buf, car(m), DIGITS_PER_WORD));
		m = cdr(m);
	}
	pr("e");
	if (e+n_digits-1 >= 0)
		pr("+");
	pr(ntoa(buf, e+n_digits-1, 0));
	return 1;
}

/*
 * Real Number Arithmetics
 */

cell integer_argument(char *who, cell x) {
	cell	n;
	char	msg[100];

	if (real_p(x)) {
		n = real_to_bignum(x);
		if (n == NIL) {
			sprintf(msg, "%s: expected integer, got", who);
			error(msg, x);
			return NIL;
		}
		return n;
	}
	return x;
}

cell make_real(int flags, cell exp, cell mant) {
	cell	n;

	n = new_atom(exp, mant);
	n = new_atom(flags, n);
	return new_atom(T_REAL, n);
}

/*
 * Remove trailing zeros and move the decimal
 * point to the END of the mantissa, e.g.:
 * real_normalize(1.234e0) --> 1234e-3
 *
 * Limit the mantissa to MANTISSA_SEGMENTS
 * machine words. This may cause a loss of
 * precision.
 *
 * Also handle numeric overflow/underflow.
 */

cell real_normalize(cell x, char *who) {
	cell	m, e, r;
	int	dgs;
	char	buf[50];

	save(x);
	e = _real_exponent(x);
	m = new_atom(T_INTEGER, _real_mantissa(x));
	save(m);
	dgs = count_digits(cdr(m));
	while (dgs > MANTISSA_SIZE) {
		r = bignum_shift_right(m);
		m = car(r);
		car(Stack) = m;
		dgs--;
		e++;
	}
	while (!_bignum_zero_p(m)) {
		r = bignum_shift_right(m);
		if (!_bignum_zero_p(cdr(r)))
			break;
		m = car(r);
		car(Stack) = m;
		e++;
	}
	if (_bignum_zero_p(m))
		e = 0;
	r = new_atom(e, NIL);
	unsave(2);
	if (count_digits(r) > DIGITS_PER_WORD) {
		sprintf(buf, "%s: real number overflow",
			who? who: "internal");
		error(buf, NOEXPR);
	}
	return make_real(_real_flags(x), e, cdr(m));
}

cell bignum_to_real(cell a) {
	int	e, flags, d;
	cell	m, n;

	m = flat_copy(a, NULL);
	cadr(m) = labs(cadr(m));
	e = 0;
	if (length(cdr(m)) > MANTISSA_SEGMENTS) {
		d = count_digits(cdr(m));
		while (d > MANTISSA_SIZE) {
			m = car(bignum_shift_right(m));
			e++;
			d--;
		}
	}
	flags = _bignum_negative_p(a)? REAL_NEGATIVE: 0;
	n = make_real(flags, e, cdr(m));
	return real_normalize(n, NULL);
}

cell real_negate(cell a) {
	if (integer_p(a))
		return bignum_negate(a);
	return _real_negate(a);
}

cell real_negative_p(cell a) {
	if (integer_p(a))
		return _bignum_negative_p(a);
	return _real_negative_p(a);
}

cell real_positive_p(cell a) {
	if (integer_p(a))
		return _bignum_positive_p(a);
	return _real_positive_p(a);
}

cell real_zero_p(cell a) {
	if (integer_p(a))
		return _bignum_zero_p(a);
	return _real_zero_p(a);
}

cell real_abs(cell a) {
	if (integer_p(a))
		return bignum_abs(a);
	if (_real_negative_p(a))
		return _real_negate(a);
	return a;
}

int real_equal_p(cell a, cell b) {
	cell	ma, mb;

	if (integer_p(a) && integer_p(b))
		return bignum_equal_p(a, b);
	if (integer_p(a))
		a = bignum_to_real(a);
	if (integer_p(b)) {
		save(a);
		b = bignum_to_real(b);
		unsave(1);
	}
	if (_real_exponent(a) != _real_exponent(b))
		return 0;
	if (_real_zero_p(a) && _real_zero_p(b))
		return 1;
	if (_real_negative_p(a) != _real_negative_p(b))
		return 0;
	ma = _real_mantissa(a);
	mb = _real_mantissa(b);
	while (ma != NIL && mb != NIL) {
		if (car(ma) != car(mb))
			return 0;
		ma = cdr(ma);
		mb = cdr(mb);
	}
	if (ma != mb)
		return 0;
	return 1;
}

/*
 * Scale the number R so that it gets exponent DESIRED_E
 * without changing its value. When there is not enough
 * room for scaling the mantissa of R, return NIL.
 * E.g.: scale_mantissa(1.0e0, -2, 0) --> 100.0e-2
 *
 * Allow the mantissa to grow to MAX_SIZE segments.
 */

cell scale_mantissa(cell r, cell desired_e, int max_size) {
	int	dgs;
	cell	n, e;

	dgs = count_digits(_real_mantissa(r));
	if (max_size && (max_size - dgs < _real_exponent(r) - desired_e))
		return NIL;
	n = new_atom(T_INTEGER, flat_copy(_real_mantissa(r), NULL));
	save(n);
	e = _real_exponent(r);
	while (e > desired_e) {
		n = bignum_shift_left(n, 0);
		car(Stack) = n;
		e--;
	}
	unsave(1);
	return make_real(_real_flags(r), e, cdr(n));
}

void autoscale(cell *pa, cell *pb) {
	if (_real_exponent(*pa) < _real_exponent(*pb)) {
		*pb = scale_mantissa(*pb, _real_exponent(*pa),
					MANTISSA_SIZE*2);
		return;
	}
	if (_real_exponent(*pa) > _real_exponent(*pb)) {
		*pa = scale_mantissa(*pa, _real_exponent(*pb),
					MANTISSA_SIZE*2);
	}
}

int real_less_p(cell a, cell b) {
	cell	ma, mb;
	int	ka, kb, neg;
	int	dpa, dpb;

	if (integer_p(a) && integer_p(b))
		return bignum_less_p(a, b);
	if (integer_p(a))
		a = bignum_to_real(a);
	if (integer_p(b)) {
		save(a);
		b = bignum_to_real(b);
		unsave(1);
	}
	if (_real_negative_p(a) && !_real_negative_p(b)) return 1;
	if (_real_negative_p(b) && !_real_negative_p(a)) return 0;
	if (_real_zero_p(a) && _real_positive_p(b)) return 1;
	if (_real_zero_p(b) && _real_positive_p(a)) return 0;
	neg = _real_negative_p(a);
	dpa = count_digits(_real_mantissa(a)) + _real_exponent(a);
	dpb = count_digits(_real_mantissa(b)) + _real_exponent(b);
	if (dpa < dpb) return neg? 0: 1;
	if (dpa > dpb) return neg? 1: 0;
	Tmp = b;
	save(a);
	save(b);
	Tmp = NIL;
	autoscale(&a, &b);
	unsave(2);
	if (a == NIL) return neg? 1: 0;
	if (b == NIL) return neg? 0: 1;
	ma = _real_mantissa(a);
	mb = _real_mantissa(b);
	ka = length(ma);
	kb = length(mb);
	if (ka < kb) return 1;
	if (ka > kb) return 0;
	while (ma != NIL) {
		if (car(ma) < car(mb)) return neg? 0: 1;
		if (car(ma) > car(mb)) return neg? 1: 0;
		ma = cdr(ma);
		mb = cdr(mb);
	}
	return 0;
}

cell real_add(cell a, cell b) {
	cell	r, m, e, aa, ab;
	int	flags, nega, negb;

	if (integer_p(a) && integer_p(b))
		return bignum_add(a, b);
	if (integer_p(a))
		a = bignum_to_real(a);
	save(a);
	if (integer_p(b))
		b = bignum_to_real(b);
	save(b);
	if (_real_zero_p(a)) {
		unsave(2);
		return b;
	}
	if (_real_zero_p(b)) {
		unsave(2);
		return a;
	}
	autoscale(&a, &b);
	if (a == NIL || b == NIL) {
		ab = real_abs(car(Stack));
		save(ab);
		aa = real_abs(caddr(Stack));
		unsave(1);
		b = unsave(1);
		a = unsave(1);
		return real_less_p(aa, ab)? b: a;
	}
	cadr(Stack) = a;
	car(Stack) = b;
	e = _real_exponent(a);
	nega = _real_negative_p(a);
	negb = _real_negative_p(b);
	a = new_atom(T_INTEGER, _real_mantissa(a));
	if (nega)
		a = bignum_negate(a);
	cadr(Stack) = a;
	b = new_atom(T_INTEGER, _real_mantissa(b));
	if (negb)
		b = bignum_negate(b);
	car(Stack) = b;
	m = bignum_add(a, b);
	unsave(2);
	flags = _bignum_negative_p(m)? REAL_NEGATIVE: 0;
	r = make_real(flags, e, cdr(bignum_abs(m)));
	return real_normalize(r, "+");
}

cell real_subtract(cell a, cell b) {
	cell	r;

	if (integer_p(b))
		b = bignum_negate(b);
	else
		b = _real_negate(b);
	save(b);
	r = real_add(a, b);
	unsave(1);
	return r;
}

cell real_multiply(cell a, cell b) {
	cell	r, m, e, ma, mb, ea, eb, neg;

	if (integer_p(a) && integer_p(b))
		return bignum_multiply(a, b);
	if (integer_p(a))
		a = bignum_to_real(a);
	save(a);
	if (integer_p(b))
		b = bignum_to_real(b);
	save(b);
	neg = _real_negative_flag(a) != _real_negative_flag(b);
	ea = _real_exponent(a);
	eb = _real_exponent(b);
	ma = new_atom(T_INTEGER, _real_mantissa(a));
	cadr(Stack) = ma;
	mb = new_atom(T_INTEGER, _real_mantissa(b));
	car(Stack) = mb;
	e = ea + eb;
	m = bignum_multiply(ma, mb);
	unsave(2);
	r = make_real(neg? REAL_NEGATIVE: 0, e, cdr(m));
	return real_normalize(r, "*");
}

cell real_divide(cell x, cell a, cell b) {
	cell	r, m, e, ma, mb, ea, eb, neg;
	int	nd, dd;

	if (integer_p(a))
		a = bignum_to_real(a);
	if (_real_zero_p(a)) {
		return make_real(0, 0, cdr(make_integer(0)));
	}
	save(a);
	if (integer_p(b))
		b = bignum_to_real(b);
	save(b);
	neg = _real_negative_flag(a) != _real_negative_flag(b);
	ea = _real_exponent(a);
	eb = _real_exponent(b);
	ma = new_atom(T_INTEGER, _real_mantissa(a));
	cadr(Stack) = ma;
	mb = new_atom(T_INTEGER, _real_mantissa(b));
	car(Stack) = mb;
	if (_bignum_zero_p(mb)) {
		unsave(2);
		return NAN;
	}
	nd = count_digits(cdr(ma));
	dd = MANTISSA_SIZE + count_digits(cdr(mb));
	while (nd < dd) {
		ma = bignum_shift_left(ma, 0);
		cadr(Stack) = ma;
		nd++;
		ea--;
	}
	e = ea - eb;
	m = bignum_divide(NOEXPR, ma, mb);
	unsave(2);
	r = make_real(neg? REAL_NEGATIVE: 0, e, cdar(m));
	return real_normalize(r, "/");
}

cell real_to_bignum(cell r) {
	cell	n;
	int	neg;

	if (_real_exponent(r) >= 0) {
		neg = _real_negative_p(r);
		n = scale_mantissa(r, 0, 0);
		if (n == NIL)
			return NIL;
		n = new_atom(T_INTEGER, _real_mantissa(n));
		if (neg)
			n = bignum_negate(n);
		return n;
	}
	return NIL;
}

cell real_integer_p(cell x) {
	if (integer_p(x))
		return 1;
	if (real_p(x) && real_to_bignum(x) != NIL)
		return 1;
	return 0;
}

cell pp_divide(cell x) {
	cell	a, expr;

	expr = x;
	x = cdr(x);
	if (cdr(x) == NIL) {
		a = make_integer(1);
		save(a);
		a = real_divide(expr, a, car(x));
		unsave(1);
		return a;
	}
	a = car(x);
	x = cdr(x);
	save(a);
	while (x != NIL) {
		if (!number_p(car(x))) {
			unsave(1);
			return error("/: expected number, got", car(x));
		}
		a = real_divide(expr, a, car(x));
		car(Stack) = a;
		x = cdr(x);
	}
	unsave(1);
	return a;
}

cell pp_exact_to_inexact(cell x) {
	cell	n;
	int	flags;

	x = cadr(x);
	if (integer_p(x)) {
		flags = _bignum_negative_p(x)? REAL_NEGATIVE: 0;
		n = make_real(flags, 0, cdr(bignum_abs(x)));
		return real_normalize(n, "exact->inexact");
	}
	return x;
}

cell pp_exact_p(cell x) {
	return integer_p(cadr(x))? TRUE: FALSE;
}

cell pp_exponent(cell x) {
	if (integer_p(cadr(x)))
		return make_integer(0);
	return make_integer(_real_exponent(cadr(x)));
}

cell pp_floor(cell x) {
	cell	n, m, e;

	x = cadr(x);
	e = _real_exponent(x);
	if (e >= 0)
		return x;
	m = new_atom(T_INTEGER, _real_mantissa(x));
	save(m);
	while (e < 0) {
		m = bignum_shift_right(m);
		m = car(m);
		car(Stack) = m;
		e++;
	}
	if (_real_negative_p(x)) {
		/* sign not in mantissa! */
		m = bignum_add(m, make_integer(1));
	}
	unsave(1);
	n = make_real(_real_flags(x), e, cdr(m));
	return real_normalize(n, "floor");
}

cell pp_inexact_p(cell x) {
	return real_p(cadr(x))? TRUE: FALSE;
}

cell pp_inexact_to_exact(cell x) {
	cell	n;

	x = cadr(x);
	if (integer_p(x))
		return x;
	n = real_to_bignum(x);
	if (n != NIL)
		return n;
	return error("inexact->exact: no exact representation for", x);
}

cell pp_mantissa(cell x) {
	cell	m;

	if (integer_p(cadr(x)))
		return cadr(x);
	m = new_atom(T_INTEGER, _real_mantissa(cadr(x)));
	if (_real_negative_p(cadr(x)))
		m = bignum_negate(m);
	return m;
}

cell pp_real_p(cell x) {
	return number_p(cadr(x))? TRUE: FALSE;
}

