# https://www.codingame.com/training/medium/mayan-calculation


def read_problem():
    mayan_dialect = []
    width, height = [int(i) for i in input().split()]

    ascii_representation = []
    for _ in range(height):
        line = input()
        ascii_representation.append(line)

    first_number_ascii = []
    s1 = int(input())
    for _ in range(s1):
        line = input()
        first_number_ascii.append(line)

    second_number_ascii = []
    s2 = int(input())
    for _ in range(s2):
        line = input()
        second_number_ascii.append(line)

    operation = input()

    return width, height, ascii_representation, first_number_ascii, second_number_ascii, operation


def build_mayan_dialect(ascii_representation):
    width = len(ascii_representation[0]) // 20
    height = len(ascii_representation)
    mayan_dialect = [""] * 20
    for i in range(20):
        for j in range(height):
            mayan_dialect[i] += ascii_representation[j][i * width : (i + 1) * width]

    return {
        ascii_repr : i for i, ascii_repr in enumerate(mayan_dialect)
    }


def main():
    width, height, ascii_representation, first_number_ascii, second_number_ascii, operation = read_problem()
    mayan_dialect = build_mayan_dialect(ascii_representation)

    def parse_number(ascii_lines):
        number = 0

        num_digits = len(ascii_lines) // height
        exponent = num_digits - 1
        for i in range(0, num_digits):
            digit_ascii = ""
            for j in range(height):
                digit_ascii += ascii_lines[i * height + j]
            digit_value = mayan_dialect[digit_ascii]
            number += digit_value * (20 ** exponent)
            exponent -= 1

        return number

    first_number = parse_number(first_number_ascii)
    second_number = parse_number(second_number_ascii)

    if operation == "+":
        result_number = first_number + second_number
    elif operation == "-":
        result_number = first_number - second_number
    elif operation == "*":
        result_number = first_number * second_number
    elif operation == "/":
        result_number = first_number // second_number

    # output result in mayan numeral system
    output_ascii_lines = []
    num_digits = result_number // 20 + 1 if result_number % 20 != 0 else result_number // 20

    mayan_dialect_reversedict = {v: k for k, v in mayan_dialect.items()}

    if result_number == 0:
        output_ascii_lines = [mayan_dialect_reversedict[0][i * width : (i + 1) * width] for i in range(height)]

    else:
        digits = []
        while result_number > 0:
            digits.append(result_number % 20)
            result_number //= 20
        digits.reverse()

        for digit in digits:
            digit_ascii = mayan_dialect_reversedict[digit]
            for i in range(height):
                output_ascii_lines.append(digit_ascii[i * width : (i + 1) * width])

    for line in output_ascii_lines:
        print(line)


if __name__ == "__main__":
    main()
