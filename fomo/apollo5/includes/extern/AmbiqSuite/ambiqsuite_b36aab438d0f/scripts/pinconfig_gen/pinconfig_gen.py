import argparse
import csv


# ******************************************************************************
#
# Main function.
#
# ******************************************************************************
def main():

    S = ''

    #
    # Read the arguments from the command line.
    #
    args = parse_arguments()

    #
    # Open the template file
    #
    with open(args.template) as tempfile:
        template = ''.join(line for line in tempfile)

    #
    # Open the CSV file containing the pin information, and convert it to a set
    # of dictionaries.
    #
    pin_array = csv_to_dict_array(args.csv)

    #
    # Generate macros for every pin in the spreadsheet.
    #
    S = '\n\n'.join(print_macros(pin,int(args.apollohal)) for pin in pin_array)

    #
    # Construct Apollo hal name and pin group for Doxygen.
    #
    apollohal = 'apollo' + args.apollohal + '_hal'
    pingroup = 'pin' + args.apollohal
    pindesc = 'PIN definitions for Apollo' + args.apollohal + '.'
    pinfname = args.output

    #
    # If the old style HAL API, create the needed helper macros.
    #
    if int(args.apollohal) < 3:
        header_macros =                                                                         \
        '\n'                                                                                    \
        '//*****************************************************************************\n'     \
        '//\n'                                                                                  \
        '// Pin definition helper macros.\n'                                                    \
        '//\n'                                                                                  \
        '//*****************************************************************************\n'     \
        '#define AM_HAL_PIN_DIR_INPUT      (AM_HAL_GPIO_INPEN)\n'                               \
        '#define AM_HAL_PIN_DIR_OUTPUT     (AM_HAL_GPIO_OUT_PUSHPULL)\n'                        \
        '#define AM_HAL_PIN_DIR_OPENDRAIN  (AM_HAL_GPIO_OUT_OPENDRAIN)\n'                       \
        '#define AM_HAL_PIN_DIR_3STATE     (AM_HAL_GPIO_OUT_3STATE)\n'                          \
        '\n'                                                                                    \
        '//*****************************************************************************\n'     \
        '//\n'                                                                                  \
        '// Pin definition helper macros.\n'                                                    \
        '//\n'                                                                                  \
        '//*****************************************************************************\n'     \
        '#define AM_HAL_PIN_DISABLE        (AM_HAL_GPIO_FUNC(3))\n'                             \
        '#define AM_HAL_PIN_INPUT          (AM_HAL_GPIO_FUNC(3) | AM_HAL_PIN_DIR_INPUT)\n'      \
        '#define AM_HAL_PIN_OUTPUT         (AM_HAL_GPIO_FUNC(3) | AM_HAL_PIN_DIR_OUTPUT)\n'     \
        '#define AM_HAL_PIN_OPENDRAIN      (AM_HAL_GPIO_FUNC(3) | AM_HAL_PIN_DIR_OPENDRAIN)\n'  \
        '#define AM_HAL_PIN_3STATE         (AM_HAL_GPIO_FUNC(3) | AM_HAL_PIN_DIR_3STATE)\n'
    else:
        header_macros = ''


    #
    # Substitute the pin information into the template.
    #
    output = template.format(pindef=S,
                             apollohal=apollohal,
                             pingroup=pingroup,
                             pindesc=pindesc,
                             pinfname=pinfname,
                             header_macros=header_macros)

    if args.output is None:
        print(output, end='')
    else:
        with open(args.output, 'w', newline='\n') as f:
            print(output, file=f, end='')


# ******************************************************************************
#
# Generate commmand line options, and parse arguments.
#
# ******************************************************************************
def parse_arguments():

    parser = argparse.ArgumentParser()

    parser.add_argument('csv', help='Name of the pin definition csv file.')
    parser.add_argument('template', help='Name of the template file.')
    parser.add_argument('-o', '--output', dest='output', help='Output file')
    parser.add_argument('-a', '--apollohal', dest='apollohal', default='2', help='Output file')

    return parser.parse_args()


# ******************************************************************************
#
# Transform the CSV into a dictionary using the first row as keys for the other
# rows.
#
# ******************************************************************************
def csv_to_dict_array(csvfile):

    allrows = []
    with open(csvfile) as f:
        csvreader = csv.reader(f)
        allrows = [row for row in csvreader]

    keys = allrows[0]

    array = [dict(zip(keys, row)) for row in allrows[1:]]

    return array


# ******************************************************************************
#
# Print all of the macros for the given pin dictionary
#
# ******************************************************************************
def print_macros(pindict, apollohal):

    # Print all of the macros for functions 0 through 7, filtering out any
    # function that shows up as an empty string.
    numbered_funcs = [print_function_macro(pindict, n, apollohal) for n in range(8)]

    # Add in the power column if it exists
    all_funcs = numbered_funcs + [print_function_macro(pindict, 'POWER', apollohal)]

    # Filter out any function selectgs that didn't exist
    filtered_list = filter(None, all_funcs)

    # The filtered list will serve as out starting point for the block of GPIO
    # configuration strings.
    S = '\n'.join(filtered_list)

    #
    # Package availability information should be found in the 'PACKAGES' column.
    #
    packages = pindict['PACKAGES'].split()
    if 'ALL' not in packages:
        #
        # If this pin isn't on every package, surround the pin configuration
        # macros with a conditional statement listing the names of the packages
        # where it is included.
        #
        startdef = '#if ' + ' || '.join('defined (AM_PACKAGE_{})'.format(x) for x in packages) + '\n'
        enddef = '\n' + '#endif // ' + ' || '.join('defined (AM_PACKAGE_{})'.format(x) for x in packages)
        S = startdef + S + enddef

    return S


# ******************************************************************************
#
# Print function macro for a numbered row
#
# ******************************************************************************
def print_function_macro(pindict, funcsel, apollohal):

    # Figure out what the normal function select string is.
    function = pindict[str(funcsel)]

    if function == '':
        return None

    funcnumber = funcsel

    if funcnumber == 'POWER':
        funcnumber = 3

    #
    # Start building the macro value with the function select number
    #

    macrocomment  = ''
    if apollohal < 3:
        macrovalue = '(AM_HAL_GPIO_FUNC({n})'.format(n=funcnumber)

        #
        # Add in terms for the pin configuration options based on the bracketed
        # fields in the function cell.
        #
        if '[3]' in function:
            macrovalue += ' | AM_HAL_GPIO_INPEN'
        if '[OD]' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_OPENDRAIN | AM_HAL_GPIO_INPEN'
        if '[I]' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_INPUT'
        if '[A]' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_INPUT'
        if '[B]' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_INPUT'
        if 'PSOURCE' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_OUTPUT | AM_HAL_GPIO_POWERSOURCE'
            # For power switches, prepend a comment about usage
            macrocomment = '// PSOURCE usage in pushpull: GPIOWT=1 to activate the power switch, GPIOWT=0 to disable\n'
        if 'PSINK' in function:
            macrovalue += ' | AM_HAL_PIN_DIR_OPENDRAIN | AM_HAL_GPIO_POWERSINK'
            # For power switches, prepend a comment about usage
            macrocomment = '// PSINK usage: GPIOWT=0 to activate the power switch, GPIOWT=1 to disable\n'

        # Close out the macro value with an ending parenthesis
        macrovalue += ')'
    else:
        macrovalue = '({n})'.format(n=funcnumber)

    #
    # Fix up the function name, either by removing the bracketed options,
    # or by removing the number after the "GPIO"
    #
    if function.startswith('GPIO'):
        function = 'GPIO'

    function = function.split(' ')[0]

    #
    # Build the "define" side of the macro using both the pin number and
    # the function name.
    #
    macroname = '#define AM_HAL_PIN_{pin}_{func}'.format(pin=pindict['PIN'], func=function)

    # Return the full macro line.
    return macrocomment+'{macroname:33} {macrovalue}'.format(macroname=macroname, macrovalue=macrovalue)


if __name__ == '__main__':
    main()
