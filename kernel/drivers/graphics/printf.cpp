/** 
 * Copied from https://github.com/eyalroz/printf with modifications
 */
#include <drivers/graphics/printf.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

#define PRINTF_DECIMAL_BUFFER_SIZE 32
#define PRINTF_INTEGER_BUFFER_SIZE 32
#define PRINTF_DEFAULT_FLOAT_PRECISION 6
#define PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9
#define PRINTF_LOG10_TAYLOR_TERMS 4

#define PRINTF_PREFER_DECIMAL false
#define PRINTF_PREFER_EXPONENTIAL true

#define PRINTF_CONCATENATE(s1, s2) s1##s2
#define PRINTF_EXPAND_THEN_CONCATENATE(s1, s2) PRINTF_CONCATENATE(s1, s2)
#define PRINTF_FLOAT_NOTATION_THRESHOLD \
    PRINTF_EXPAND_THEN_CONCATENATE(1e, PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL)

// internal flag definitions
#define FLAGS_ZEROPAD (1U << 0U)
#define FLAGS_LEFT (1U << 1U)
#define FLAGS_PLUS (1U << 2U)
#define FLAGS_SPACE (1U << 3U)
#define FLAGS_HASH (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_CHAR (1U << 6U)
#define FLAGS_SHORT (1U << 7U)
#define FLAGS_INT (1U << 8U)
#define FLAGS_LONG (1U << 9U)
#define FLAGS_LONG_LONG (1U << 10U)
#define FLAGS_PRECISION (1U << 11U)
#define FLAGS_ADAPT_EXP (1U << 12U)
#define FLAGS_POINTER (1U << 13U)
#define FLAGS_SIGNED (1U << 14U)

typedef unsigned int printf_flags_t;

#define BASE_BINARY 2
#define BASE_OCTAL 8
#define BASE_DECIMAL 10
#define BASE_HEX 16

typedef uint8_t numeric_base_t;
typedef unsigned long long printf_unsigned_value_t;
typedef unsigned long long printf_signed_value_t;
typedef unsigned int printf_size_t;

#define PRINTF_MAX_POSSIBLE_BUFFER_SIZE INT_MAX

#define DOUBLE_SIZE_IN_BITS 64
typedef uint64_t double_uint_t;
#define DOUBLE_EXPONENT_MASK 0x7FFU
#define DOUBLE_BASE_EXPONENT 1023
#define DOUBLE_MAX_SUBNORMAL_EXPONENT_OF_10 -308
#define DOUBLE_MAX_SUBNORMAL_POWER_OF_10 1e-308

#define DOUBLE_STORED_MANTISSA_BITS (DBL_MANT_DIG - 1)

typedef union {
    double_uint_t U;
    double F;
} double_with_bit_access;

static inline double_with_bit_access get_bit_access(double X) {
    double_with_bit_access dwba;
    dwba.F = X;
    return dwba;
}

static inline int get_sign_bit(double x) {
    // The sign is stored in the highest bit
    return static_cast<int>(get_bit_access(x).U >> (DOUBLE_SIZE_IN_BITS - 1));
}

static inline int get_exp2(double_with_bit_access x) {
    // The exponent in an IEEE-754 floating-point number occupies a contiguous
    // sequence of bits, but with a non-trivial representation: An
    // unsigned offset from some negative value.
    return static_cast<int>((x.U >> DOUBLE_STORED_MANTISSA_BITS) &
                            DOUBLE_EXPONENT_MASK) -
           DOUBLE_BASE_EXPONENT;
}

#define PRINTF_ABS(x) ((x) > 0 ? (x) : -(x))
#define ABS_FOR_PRINTING(x) \
    ((printf_unsigned_value_t)(x > 0 ? (x) : -((printf_signed_value_t)x)))

// wrapper (used as buffer) for output function type
struct output_gadget_t {
    void (*function)(char c, void* extra_arg);
    void* extra_function_arg;
    char* buffer;
    printf_size_t pos;
    printf_size_t max_chars;
};

static inline void putchar_via_gadget(output_gadget_t* gadget, char c) {
    printf_size_t write_pos = gadget->pos++;

    if (write_pos >= gadget->max_chars)
        return;
    if (gadget->function != NULL)
        gadget->function(c, gadget->extra_function_arg);
    else
        gadget->buffer[write_pos] = c;
}

static inline void append_termination_with_gadget(output_gadget_t* gadget) {
    if (gadget->function != NULL || gadget->max_chars == 0)
        return;
    if (gadget->buffer == NULL)
        return;

    printf_size_t null_char_pos =
        gadget->pos < gadget->max_chars ? gadget->pos : gadget->max_chars - 1;
    gadget->buffer[null_char_pos] = '\0';
}

static inline void putchar_wrapper(char c, void* unused) {
    static_cast<void>(unused);
    putchar(c);
}

static inline output_gadget_t discarding_gadget() {
    output_gadget_t gadget;

    gadget.function = NULL;
    gadget.extra_function_arg = NULL;
    gadget.buffer = NULL;
    gadget.pos = 0;
    gadget.max_chars = 0;

    return gadget;
}

static inline output_gadget_t buffer_gadget(char* buffer,
                                            printf_size_t buffer_size) {
    printf_size_t usable_buffer_size =
        (buffer_size > PRINTF_MAX_POSSIBLE_BUFFER_SIZE)
            ? PRINTF_MAX_POSSIBLE_BUFFER_SIZE
            : buffer_size;
    output_gadget_t result = discarding_gadget();

    if (buffer != NULL) {
        result.buffer = buffer;
        result.max_chars = usable_buffer_size;
    }

    return result;
}

static inline output_gadget_t function_gadget(void (*function)(char, void*),
                                              void* extra_arg) {
    output_gadget_t result = discarding_gadget();

    result.function = function;
    result.extra_function_arg = extra_arg;
    result.max_chars = PRINTF_MAX_POSSIBLE_BUFFER_SIZE;

    return result;
}

static inline output_gadget_t extern_putchar_gadget() {
    return function_gadget(putchar_wrapper, NULL);
}

static inline printf_size_t strnlen_s(const char* str, printf_size_t maxsize) {
    const char* s;
    for (s = str; *s && maxsize--; ++s)
        ;
    return static_cast<printf_size_t>(s - str);
}

static inline bool is_digit(char ch) {
    return (ch >= '0') && (ch <= '9');
}

static printf_size_t atou(const char** str) {
    printf_size_t i = 0U;
    while (is_digit(**str))
        i = i * 10U + static_cast<printf_size_t>(*((*str)++) - '0');
    return i;
}

static void out_rev(output_gadget_t* output, const char* buf, printf_size_t len,
                    printf_size_t width, printf_flags_t flags) {
    const printf_size_t start_pos = output->pos;

    // pad spaces up to given width
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD))
        for (printf_size_t i = len; i < width; i++)
            putchar_via_gadget(output, ' ');

    // reverse string
    while (len)
        putchar_via_gadget(output, buf[--len]);

    if (flags & FLAGS_LEFT)
        while (output->pos - start_pos < width)
            putchar_via_gadget(output, ' ');
}

static void print_integer_finalization(output_gadget_t* output, char* buf,
                                       printf_size_t len, bool negative,
                                       numeric_base_t base,
                                       printf_size_t precision,
                                       printf_size_t width,
                                       printf_flags_t flags) {
    printf_size_t unpadded_len = len;

    // pad with leading zeros
    {
        if (!(flags & FLAGS_LEFT)) {
            if (width && (flags & FLAGS_ZEROPAD) &&
                (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
                width--;
            while ((flags & FLAGS_ZEROPAD) && (len < width) &&
                   (len < PRINTF_INTEGER_BUFFER_SIZE))
                buf[len++] = '0';
        }

        while ((len < precision) && (len < PRINTF_INTEGER_BUFFER_SIZE))
            buf[len++] = '0';

        if (base == BASE_OCTAL && (len > unpadded_len)) {
            flags &= ~FLAGS_HASH;
        }
    }

    // handle hash
    if (flags & (FLAGS_HASH | FLAGS_POINTER)) {
        if (!(flags & FLAGS_PRECISION) && len &&
            ((len == precision) || (len == width))) {
            if (unpadded_len < len)
                len--;
            if (len && (base == BASE_HEX || base == BASE_BINARY) &&
                (unpadded_len < len))
                len--;
        }

        if ((base == BASE_HEX) && !(flags & FLAGS_UPPERCASE) &&
            (len < PRINTF_INTEGER_BUFFER_SIZE))
            buf[len++] = 'x';
        else if ((base == BASE_HEX) && (flags & FLAGS_UPPERCASE) &&
                 (len < PRINTF_INTEGER_BUFFER_SIZE))
            buf[len++] = 'X';
        else if ((base == BASE_BINARY) && (len < PRINTF_INTEGER_BUFFER_SIZE))
            buf[len++] = 'b';
        if (len < PRINTF_INTEGER_BUFFER_SIZE)
            buf[len++] = '0';
    }

    if (len < PRINTF_INTEGER_BUFFER_SIZE) {
        if (negative)
            buf[len++] = '-';
        else if (flags & FLAGS_PLUS)
            buf[len++] = '+';
        else if (flags & FLAGS_SPACE)
            buf[len++] = ' ';
    }

    out_rev(output, buf, len, width, flags);
}

static void print_integer(output_gadget_t* output,
                          printf_unsigned_value_t value, bool negative,
                          numeric_base_t base, printf_size_t precision,
                          printf_size_t width, printf_flags_t flags) {
    char buf[PRINTF_INTEGER_BUFFER_SIZE];
    printf_size_t len = 0U;

    if (!value) {
        if (!(flags & FLAGS_PRECISION)) {
            buf[len++] = '0';
            flags &= ~FLAGS_HASH;
        } else if (base == BASE_HEX) {
            flags &= ~FLAGS_HASH;
        }
    } else {
        do {
            const char digit = static_cast<char>(value % base);
            buf[len++] = static_cast<char>(
                digit < 10
                    ? '0' + digit
                    : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10);
            value /= base;
        } while (value && (len < PRINTF_INTEGER_BUFFER_SIZE));
    }

    print_integer_finalization(output, buf, len, negative, base, precision,
                               width, flags);
}

struct double_components {
    int_fast64_t integral;
    int_fast64_t fractional;
    bool is_negative;
};

#define NUM_DECIMAL_DIGITS_IN_INT64_T 18
#define PRINTF_MAX_PRECOMPUTED_POWER_OF_10 NUM_DECIMAL_DIGITS_IN_INT64_T
static const double powers_of_10[NUM_DECIMAL_DIGITS_IN_INT64_T] = {
    1e00, 1e01, 1e02, 1e03, 1e04, 1e05, 1e06, 1e07, 1e08,
    1e09, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17};

#define PRINTF_MAX_SUPPORTED_PRECISION NUM_DECIMAL_DIGITS_IN_INT64_T - 1

static double_components get_components(double number,
                                        printf_size_t precision) {
    double_components number_;
    number_.is_negative = get_sign_bit(number);
    double abs_number = (number_.is_negative) ? -number : number;
    number_.integral = static_cast<int_fast64_t>(abs_number);
    double remainder = (abs_number - static_cast<double>(number_.integral) *
                                         powers_of_10[precision]);
    number_.fractional = static_cast<int_fast64_t>(remainder);

    remainder -= static_cast<double>(number_.fractional);

    if (remainder > 0.5) {
        ++number_.fractional;
        if (static_cast<double>(number_.fractional) >=
            powers_of_10[precision]) {
            number_.fractional = 0;
            ++number_.integral;
        }
    } else if ((remainder == 0.5) &&
               ((number_.fractional == 0U) || (number_.fractional & 1U))) {
        ++number_.fractional;
    }

    if (precision == 0U) {
        remainder = abs_number - static_cast<double>(number_.integral);
        if ((!(remainder < 0.5) || (remainder > 0.5)) && (number_.integral & 1))
            ++number_.integral;
    }

    return number_;
}

struct scaling_factor {
    double raw_factor;
    bool multiply;
};

static double apply_scaling(double num, scaling_factor normalization) {
    return normalization.multiply ? num * normalization.raw_factor
                                  : num / normalization.raw_factor;
}

static double unapply_scaling(double normalized, scaling_factor normalization) {
    return normalization.multiply ? normalized / normalization.raw_factor
                                  : normalized * normalization.raw_factor;
}

static scaling_factor update_normalization(scaling_factor sf,
                                           double extra_multiplicative_factor) {
    scaling_factor result;

    if (sf.multiply) {
        result.multiply = true;
        result.raw_factor = sf.raw_factor * extra_multiplicative_factor;
    } else {
        int factor_exp2 = get_exp2(get_bit_access(sf.raw_factor));
        int extra_factor_exp2 =
            get_exp2(get_bit_access(extra_multiplicative_factor));

        if (PRINTF_ABS(factor_exp2) > PRINTF_ABS(extra_factor_exp2)) {
            result.multiply = false;
            result.raw_factor = sf.raw_factor / extra_multiplicative_factor;
        } else {
            result.multiply = true;
            result.raw_factor = extra_multiplicative_factor / sf.raw_factor;
        }
    }

    return result;
}

static double_components get_normalized_components(bool negative,
                                                   printf_size_t precision,
                                                   double non_normalized,
                                                   scaling_factor normalization,
                                                   int floored_exp10) {
    double_components components;
    components.is_negative = negative;
    double scaled = apply_scaling(non_normalized, normalization);

    bool close_to_representation_extremum =
        ((-floored_exp10 + static_cast<int>(precision) > -DBL_MAX_10_EXP - 1));
    if (close_to_representation_extremum)
        return get_components(negative ? -scaled : scaled, precision);

    components.integral = static_cast<int_fast64_t>(scaled);

    double remainder = non_normalized -
                       unapply_scaling(static_cast<double>(components.integral),
                                       normalization);
    double prec_power_of_10 = powers_of_10[precision];
    scaling_factor account_for_precision =
        update_normalization(normalization, prec_power_of_10);
    double scaled_remainder = apply_scaling(remainder, account_for_precision);
    double rounding_threshold = 0.5;

    components.fractional = static_cast<int_fast64_t>(scaled_remainder);
    scaled_remainder -= static_cast<double>(components.fractional);
    components.fractional += (scaled_remainder >= rounding_threshold);

    if (scaled_remainder == rounding_threshold)
        components.fractional &= ~(static_cast<int_fast64_t>(0x1));

    if (static_cast<double>(components.fractional) >= prec_power_of_10) {
        components.fractional = 0;
        ++components.integral;
    }

    return components;
}

static void print_broken_up_decimal(double_components number,
                                    output_gadget_t* output,
                                    printf_size_t precision,
                                    printf_size_t width, printf_flags_t flags,
                                    char* buf, printf_flags_t len) {
    if (precision != 0U) {
        // do fractional part, as an unsigned number

        printf_size_t count = precision;

        // %g/%G mandates we skip the trailing 0 digits...
        if ((flags & FLAGS_ADAPT_EXP) && !(flags & FLAGS_HASH) &&
            (number.fractional > 0)) {
            while (true) {
                int_fast64_t digit = number.fractional % 10U;
                if (digit != 0)
                    break;
                --count;
                number.fractional /= 10U;
            }
        }

        if (number.fractional > 0 || !(flags & FLAGS_ADAPT_EXP) ||
            (flags & FLAGS_HASH)) {
            while (len < PRINTF_DECIMAL_BUFFER_SIZE) {
                --count;
                buf[len++] = static_cast<char>('0' + number.fractional % 10U);
                if (!(number.fractional /= 10U))
                    break;
            }

            // add extra 0s
            while ((len < PRINTF_DECIMAL_BUFFER_SIZE) && (count > 0U)) {
                buf[len++] = '0';
                --count;
            }

            if (len < PRINTF_DECIMAL_BUFFER_SIZE)
                buf[len++] = '.';
        }
    } else {
        if ((flags & FLAGS_HASH) && (len < PRINTF_DECIMAL_BUFFER_SIZE))
            buf[len++] = '.';
    }

    // Write the integer part of the number (it comes after the fractional
    // since the character order is reversed)
    while (len < PRINTF_DECIMAL_BUFFER_SIZE) {
        buf[len++] = (char)('0' + (number.integral % 10));
        if (!(number.integral /= 10))
            break;
    }

    // pad leading zeros
    if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
        if (width &&
            (number.is_negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
            width--;
        while ((len < width) && (len < PRINTF_DECIMAL_BUFFER_SIZE))
            buf[len++] = '0';
    }

    if (len < PRINTF_DECIMAL_BUFFER_SIZE) {
        if (number.is_negative)
            buf[len++] = '-';
        else if (flags & FLAGS_PLUS)
            buf[len++] = '+';  // ignore the space if the '+' exists
        else if (flags & FLAGS_SPACE)
            buf[len++] = ' ';
    }

    out_rev(output, buf, len, width, flags);
}

static void print_decimal_number(output_gadget_t* output, double number,
                                 printf_size_t precision, printf_size_t width,
                                 printf_flags_t flags, char* buf,
                                 printf_size_t len) {
    double_components value = get_components(number, precision);
    print_broken_up_decimal(value, output, precision, width, flags, buf, len);
}

static int bastardized_floor(double x) {
    if (x >= 0)
        return static_cast<int>(x);
    int n = static_cast<int>(x);
    return (static_cast<double>(n) == x) ? n : n - 1;
}

static double log10_of_positive(double positive_number) {
    // The implementation follows David Gay (https://www.ampl.com/netlib/fp/dtoa.c).
    //
    // Since log_10 ( M * 2^x ) = log_10(M) + x , we can separate the components of
    // our input number, and need only solve log_10(M) for M between 1 and 2 (as
    // the base-2 mantissa is always 1-point-something). In that limited range, a
    // Taylor series expansion of log10(x) should serve us well enough; and we'll
    // take the mid-point, 1.5, as the point of expansion.

    double_with_bit_access dwba = get_bit_access(positive_number);
    int exp2 = get_exp2(dwba);

    dwba.U = (dwba.U &
              ((static_cast<double_uint_t>(1) << DOUBLE_STORED_MANTISSA_BITS) -
               1U)) |
             (static_cast<double_uint_t>(DOUBLE_BASE_EXPONENT)
              << DOUBLE_STORED_MANTISSA_BITS);
    double z = (dwba.F - 1.5);
    return (
        // Taylor expansion around 1.5:
        0.1760912590556812420  // Expansion term 0: ln(1.5) / ln(10)
        +
        z * 0.2895296546021678851  // Expansion term 1: (M - 1.5)   * 2/3  / ln(10)
        -
        z * z *
            0.0965098848673892950  // Expansion term 2: (M - 1.5)^2 * 2/9  / ln(10)
        +
        z * z * z *
            0.0428932821632841311  // Expansion term 2: (M - 1.5)^3 * 8/81 / ln(10)
        // exact log_2 of the exponent x, with logarithm base change
        +
        exp2 *
            0.30102999566398119521  // = exp2 * log_10(2) = exp2 * ln(2)/ln(10)
    );
}

static double pow10_of_int(int floored_exp10) {
    // a crude hack for avoiding undesired behaior with barely-normal or slightly-subnormal value
    if (floored_exp10 == DOUBLE_MAX_SUBNORMAL_EXPONENT_OF_10)
        return DOUBLE_MAX_SUBNORMAL_POWER_OF_10;

    // Compute 10^(floored_exp10) but (try to) make sure that doesn't overflow
    double_with_bit_access dwba;
    int exp2 = bastardized_floor(floored_exp10 * 3.321928094887362 + 0.5);
    const double z =
        floored_exp10 * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2 = z * z;

    dwba.U = (static_cast<double_uint_t>(exp2) + DOUBLE_BASE_EXPONENT)
             << DOUBLE_STORED_MANTISSA_BITS;
    // compute exp(z) using continued fractions
    dwba.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    return dwba.F;
}

static void print_exponential_number(output_gadget_t* output, double number,
                                     printf_size_t precision,
                                     printf_size_t width, printf_flags_t flags,
                                     char* buf, printf_size_t len) {
    const bool negative = get_sign_bit(number);
    double abs_number = negative ? -number : number;

    int floored_exp10;
    bool abs_exp10_covered_by_powers_table;
    scaling_factor normalization;

    // Determine the decimal exponent
    if (abs_number == 0.0) {
        floored_exp10 = 10;
    } else {
        double exp10 = log10_of_positive(abs_number);
        floored_exp10 = bastardized_floor(exp10);
        double p10 = pow10_of_int(floored_exp10);

        if (abs_number < p10) {
            floored_exp10--;
            p10 /= 10;
        }

        abs_exp10_covered_by_powers_table =
            PRINTF_ABS(floored_exp10) < PRINTF_MAX_PRECOMPUTED_POWER_OF_10;
        normalization.raw_factor = abs_exp10_covered_by_powers_table
                                       ? powers_of_10[PRINTF_ABS(floored_exp10)]
                                       : p10;
    }

    bool fall_back_to_decimal_only_mode = false;

    if (flags & FLAGS_ADAPT_EXP) {
        int required_significant_digits =
            (precision == 0) ? 1 : static_cast<int>(precision);
        fall_back_to_decimal_only_mode =
            (floored_exp10 >= -4 &&
             floored_exp10 < required_significant_digits);
        int precision_ = fall_back_to_decimal_only_mode
                             ? static_cast<int>(precision) - 1 - floored_exp10
                             : static_cast<int>(precision) - 1;
        precision = (precision_ > 0 ? static_cast<unsigned>(precision_) : 0U);
        flags |= FLAGS_PRECISION;
    }

    normalization.multiply =
        (floored_exp10 < 0 && abs_exp10_covered_by_powers_table);
    bool should_skip_normalization =
        (fall_back_to_decimal_only_mode || floored_exp10 == 0);
    double_components decimal_part_components =
        should_skip_normalization
            ? get_components(negative ? -abs_number : abs_number, precision)
            : get_normalized_components(negative, precision, abs_number,
                                        normalization, floored_exp10);

    if (fall_back_to_decimal_only_mode) {
        if ((flags & FLAGS_ADAPT_EXP) && floored_exp10 >= -1 &&
            decimal_part_components.integral ==
                powers_of_10[floored_exp10 + 1]) {
            floored_exp10++;
            precision--;
        }
    } else {
        if (decimal_part_components.integral >= 10) {
            floored_exp10++;
            decimal_part_components.integral = 1;
            decimal_part_components.fractional = 0;
        }
    }

    printf_size_t exp10_part_width = fall_back_to_decimal_only_mode      ? 0U
                                     : (PRINTF_ABS(floored_exp10) < 100) ? 4U
                                                                         : 5U;
    printf_size_t decimal_part_width =
        ((flags & FLAGS_LEFT) && exp10_part_width)
            ? 0U
            : ((width > exp10_part_width) ? width - exp10_part_width : 0U);
    const printf_size_t printed_exponential_start_pos = output->pos;
    print_broken_up_decimal(decimal_part_components, output, precision,
                            decimal_part_width, flags, buf, len);

    if (!fall_back_to_decimal_only_mode) {
        putchar_via_gadget(output, (flags & FLAGS_UPPERCASE) ? 'E' : 'e');
        print_integer(output, ABS_FOR_PRINTING(floored_exp10),
                      floored_exp10 < 0, 10, 0, exp10_part_width - 1,
                      FLAGS_ZEROPAD | FLAGS_PLUS);
        if (flags & FLAGS_LEFT)
            while (output->pos - printed_exponential_start_pos < width)
                putchar_via_gadget(output, ' ');
    }
}

static void print_floating_point(output_gadget_t* output, double value,
                                 printf_size_t precision, printf_size_t width,
                                 printf_flags_t flags,
                                 bool prefer_exponential) {
    char buf[PRINTF_DECIMAL_BUFFER_SIZE];
    printf_size_t len = 0U;

    if (value != value) {
        out_rev(output, "nan", 3, width, flags);
        return;
    }

    if (value < -DBL_MAX) {
        out_rev(output, "fni-", 4, width, flags);
        return;
    }

    if (value > DBL_MAX) {
        out_rev(output, (flags & FLAGS_PLUS) ? "fni+" : "fni",
                (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);
        return;
    }

    if (!prefer_exponential && ((value > PRINTF_FLOAT_NOTATION_THRESHOLD) ||
                                (value < -PRINTF_FLOAT_NOTATION_THRESHOLD))) {
        print_exponential_number(output, value, precision, width, flags, buf,
                                 len);
        return;
    }

    if (!(flags & FLAGS_PRECISION))
        precision = PRINTF_DEFAULT_FLOAT_PRECISION;

    while ((len < PRINTF_DECIMAL_BUFFER_SIZE) &&
           (precision > PRINTF_MAX_SUPPORTED_PRECISION)) {
        buf[len++] = '0';
        precision--;
    }

    if (prefer_exponential)
        print_exponential_number(output, value, precision, width, flags, buf,
                                 len);
    else
        print_decimal_number(output, value, precision, width, flags, buf, len);
}

static printf_flags_t parse_flags(const char** format) {
    printf_flags_t flags = 0U;
    do {
        switch (**format) {
            case '0':
                flags |= FLAGS_ZEROPAD;
                (*format)++;
                break;
            case '-':
                flags |= FLAGS_LEFT;
                (*format)++;
                break;
            case '+':
                flags |= FLAGS_PLUS;
                (*format)++;
                break;
            case ' ':
                flags |= FLAGS_SPACE;
                (*format)++;
                break;
            case '#':
                flags |= FLAGS_HASH;
                (*format)++;
                break;
            default:
                return flags;
        }
    } while (true);
}

static inline void format_string_loop(output_gadget_t* output,
                                      const char* format, va_list args) {
#define ADVANCE_IN_FORMAT_STRING(cptr_) \
    do {                                \
        (cptr_)++;                      \
        if (!*(cptr_))                  \
            return;                     \
    } while (0)

    while (*format) {
        if (*format != '%') {
            putchar_via_gadget(output, *format);
            format++;
            continue;
        }

        // We're parsing a format specifier: %[flags][width][.precision][length]
        ADVANCE_IN_FORMAT_STRING(format);

        printf_flags_t flags = parse_flags(&format);

        // evaluate width field
        printf_size_t width = 0U;
        if (is_digit(*format)) {
            width = static_cast<printf_size_t>(atou(&format));
        } else if (*format == '*') {
            const int w = va_arg(args, int);

            if (w < 0) {
                flags |= FLAGS_LEFT;
                width = static_cast<printf_size_t>(-w);
            } else {
                width = static_cast<printf_size_t>(w);
            }

            ADVANCE_IN_FORMAT_STRING(format);
        }

        // evaluate precision field
        printf_size_t precision = 0U;
        if (*format == '.') {
            flags |= FLAGS_PRECISION;
            ADVANCE_IN_FORMAT_STRING(format);

            if (is_digit(*format)) {
                precision = static_cast<printf_size_t>(atou(&format));
            } else if (*format == '*') {
                const int precision_ = va_arg(args, int);
                precision = precision_ > 0
                                ? static_cast<printf_size_t>(precision_)
                                : 0U;
                ADVANCE_IN_FORMAT_STRING(format);
            }
        }

        // evaluate length field
        switch (*format) {
            case 'l':
                flags |= FLAGS_LONG;
                ADVANCE_IN_FORMAT_STRING(format);
                if (*format == 'l') {
                    flags |= FLAGS_LONG_LONG;
                    ADVANCE_IN_FORMAT_STRING(format);
                }
                break;
            case 'h':
                flags |= FLAGS_SHORT;
                ADVANCE_IN_FORMAT_STRING(format);
                if (*format == 'h') {
                    flags |= FLAGS_CHAR;
                    ADVANCE_IN_FORMAT_STRING(format);
                }
                break;
            case 't':
                flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG
                                                            : FLAGS_LONG_LONG);
                ADVANCE_IN_FORMAT_STRING(format);
                break;
            case 'j':
                flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG
                                                           : FLAGS_LONG_LONG);
                ADVANCE_IN_FORMAT_STRING(format);
                break;
            case 'z':
                flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG
                                                         : FLAGS_LONG_LONG);
                ADVANCE_IN_FORMAT_STRING(format);
                break;
            default:
                break;
        }

        // evaluate specifier
        switch (*format) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b': {
                if (*format == 'd' || *format == 'i')
                    flags |= FLAGS_SIGNED;
                numeric_base_t base;
                if (*format == 'x' || *format == 'X') {
                    base = BASE_HEX;
                } else if (*format == 'o') {
                    base = BASE_OCTAL;
                } else if (*format == 'b') {
                    base = BASE_BINARY;
                } else {
                    base = BASE_DECIMAL;
                    flags &= ~FLAGS_HASH;
                }

                if (*format == 'X')
                    flags |= FLAGS_UPPERCASE;

                format++;

                // ignore '0' flag when precision is given
                if (flags & FLAGS_PRECISION)
                    flags &= ~FLAGS_ZEROPAD;

                if (flags & FLAGS_SIGNED) {
                    // a signed specifier: d, i or possibly I + bit size if enabled
                    if (flags & FLAGS_LONG_LONG) {
                        const long long value = va_arg(args, long long);
                        print_integer(output, ABS_FOR_PRINTING(value),
                                      value < 0, base, precision, width, flags);
                    } else if (flags & FLAGS_LONG) {
                        const long value = va_arg(args, long);
                        print_integer(output, ABS_FOR_PRINTING(value),
                                      value < 0, base, precision, width, flags);
                    } else {
                        const int value = (flags & FLAGS_CHAR)
                                              ? (signed char)va_arg(args, int)
                                          : (flags & FLAGS_SHORT)
                                              ? (short int)va_arg(args, int)
                                              : va_arg(args, int);
                        print_integer(output, ABS_FOR_PRINTING(value),
                                      value < 0, base, precision, width, flags);
                    }
                } else {
                    // an unsigned specifier: u, x, X, o, b
                    flags &= ~(FLAGS_PLUS | FLAGS_SPACE);

                    if (flags & FLAGS_LONG_LONG) {
                        print_integer(output,
                                      static_cast<printf_unsigned_value_t>(
                                          va_arg(args, unsigned long long)),
                                      false, base, precision, width, flags);
                    } else if (flags & FLAGS_LONG) {
                        print_integer(output,
                                      static_cast<printf_unsigned_value_t>(
                                          va_arg(args, unsigned long)),
                                      false, base, precision, width, flags);
                    } else {
                        const printf_size_t value =
                            (flags & FLAGS_CHAR) ? static_cast<uint8_t>(va_arg(
                                                       args, unsigned int))
                            : (flags & FLAGS_SHORT)
                                ? static_cast<unsigned short int>(
                                      va_arg(args, unsigned int))
                                : va_arg(args, unsigned int);
                        print_integer(
                            output, static_cast<printf_unsigned_value_t>(value),
                            false, base, precision, width, flags);
                    }
                }
                break;
            }
            case 'f':
            case 'F':
                if (*format == 'F')
                    flags |= FLAGS_UPPERCASE;
                print_floating_point(output, va_arg(args, double), precision,
                                     width, flags, PRINTF_PREFER_DECIMAL);
                format++;
                break;
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if ((*format == 'g') || (*format == 'G'))
                    flags |= FLAGS_ADAPT_EXP;
                if ((*format == 'E') || (*format == 'G'))
                    flags |= FLAGS_UPPERCASE;
                print_floating_point(output, va_arg(args, double), precision,
                                     width, flags, PRINTF_PREFER_EXPONENTIAL);
                format++;
                break;
            case 'c': {
                printf_size_t l = 1U;
                // pre padding
                if (!(flags & FLAGS_LEFT))
                    while (l++ < width)
                        putchar_via_gadget(output, ' ');
                // char support
                putchar_via_gadget(output,
                                   static_cast<char>(va_arg(args, int)));
                // post padding
                if (flags & FLAGS_LEFT)
                    while (l++ < width)
                        putchar_via_gadget(output, ' ');
                format++;
                break;
            }
            case 's': {
                const char* p = va_arg(args, char*);
                if (p == NULL) {
                    out_rev(output, ")llun(", 6, width, flags);
                } else {
                    printf_size_t l = strnlen_s(
                        p, precision ? precision
                                     : PRINTF_MAX_POSSIBLE_BUFFER_SIZE);
                    // pre padding
                    if (flags & FLAGS_PRECISION)
                        l = (l < precision ? l : precision);
                    if (!(flags & FLAGS_LEFT))
                        while (l++ < width)
                            putchar_via_gadget(output, ' ');
                    // string output
                    while ((*p != 0) &&
                           (!(flags & FLAGS_PRECISION) || precision)) {
                        putchar_via_gadget(output, *(p++));
                        --precision;
                    }
                    // post padding
                    if (flags & FLAGS_LEFT)
                        while (l++ < width)
                            putchar_via_gadget(output, ' ');
                }
                format++;
                break;
            }
            case 'p': {
                width = sizeof(void*) * 2U +
                        2;  // 2 hex chars per byte + the "0x" prefix
                flags |= FLAGS_ZEROPAD | FLAGS_POINTER;
                uintptr_t value = (uintptr_t)va_arg(args, void*);
                if (value == (uintptr_t)NULL)
                    out_rev(output, ")lin(", 5, width, flags);
                else
                    print_integer(output,
                                  static_cast<printf_unsigned_value_t>(value),
                                  false, BASE_HEX, precision, width, flags);
                format++;
                break;
            }

            case '%':
                putchar_via_gadget(output, '%');
                format++;
                break;

            case 'n': {
                if (flags & FLAGS_CHAR)
                    *(va_arg(args, char*)) = static_cast<char>(output->pos);
                else if (flags & FLAGS_SHORT)
                    *(va_arg(args, short*)) = static_cast<short>(output->pos);
                else if (flags & FLAGS_LONG)
                    *(va_arg(args, long*)) = static_cast<long>(output->pos);
                else if (flags & FLAGS_LONG_LONG)
                    *(va_arg(args, long long*)) =
                        static_cast<long long int>(output->pos);
                else
                    *(va_arg(args, int*)) = static_cast<int>(output->pos);
                format++;
                break;
            }
            default:
                putchar_via_gadget(output, *format);
                format++;
                break;
        }
    }
}

static int vsnprintf_impl(output_gadget_t* output, const char* format,
                          va_list args) {
    format_string_loop(output, format, args);
    append_termination_with_gadget(output);

    return static_cast<int>(output->pos);
}

int vprintf(const char* format, va_list arg) {
    output_gadget_t gadget = extern_putchar_gadget();
    return vsnprintf_impl(&gadget, format, arg);
}

int vsnprintf(char* s, size_t count, const char* format, va_list arg) {
    output_gadget_t gadget = buffer_gadget(s, count);
    return vsnprintf_impl(&gadget, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg) {
    return vsnprintf(s, PRINTF_MAX_POSSIBLE_BUFFER_SIZE, format, arg);
}

int vfctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
               const char* format, va_list arg) {
    output_gadget_t gadget = function_gadget(out, extra_arg);
    return vsnprintf_impl(&gadget, format, arg);
}

int printf(const char* format, ...) {
    va_list args;

    va_start(args, format);
    const int ret = vprintf(format, args);
    va_end(args);

    return ret;
}

int sprintf(char* s, const char* format, ...) {
    va_list args;

    va_start(args, format);
    const int ret = vsprintf(s, format, args);
    va_end(args);

    return ret;
}

int snprintf(char* s, size_t n, const char* format, ...) {
    va_list args;

    va_start(args, format);
    const int ret = vsnprintf(s, n, format, args);
    va_end(args);

    return ret;
}

int fctprintf(void (*out)(char c, void* extra_arg), void* extra_arg,
              const char* format, ...) {
    va_list args;

    va_start(args, format);
    const int ret = vfctprintf(out, extra_arg, format, args);
    va_end(args);

    return ret;
}